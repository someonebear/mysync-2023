#include <utime.h>

#include "mysync.h"

// Create all missing directories in path.
void create_dirs(char *existing, char *path)
{

  // part of path that already exists.
  char exists[MAXPATHLEN];

  // rest of path.
  char to_create[MAXPATHLEN];

  strcpy(exists, existing);
  strcpy(to_create, path);

  char *p = strchr(to_create, '/');
  // Base case of function. If no more directories in path, return.
  if (p == NULL)
  {
    return;
  }
  // Recursive step.
  // If first directory in "to_create" does not exist, create it.
  // If it already exists, move directory to "exists", and recurse function on rest of path.
  else
  {
    // Find first directory in "to create".
    char *first_dir = strdup(to_create);
    char *p2 = strchr(first_dir, '/');
    p2++;
    *p2 = '\0';

    // Move first directory to "exists".
    snprintf(exists, MAXPATHLEN, "%s%s", exists, first_dir);
    free(first_dir);
    if (mkdir(exists) != 0)
    {
      if (errno != EEXIST)
      {
        fprintf(stderr, "Could not create directory: %s\n", exists);
        perror(__func__);
        exit(EXIT_FAILURE);
      }
    }

    // p is rest of "to_create" after the first directory was created.
    p++;
    strcpy(to_create, p);
    create_dirs(exists, to_create);
  }
}

// Copy source to dest.
void fcopy(char *source, char *dest)
{
  FILE *fp_source = fopen(source, "r");
  FILE *fp_dest = fopen(dest, "w");
  CHECK_ALLOC(fp_source);
  CHECK_ALLOC(fp_dest);

  char buffer[1024];
  size_t got, wrote;

  while ((got = fread(buffer, 1, sizeof(*buffer), fp_source)) > 0)
  {
    if (ferror(fp_source))
    {
      fclose(fp_source);
      fclose(fp_dest);
      fprintf(stderr, "Error opening file.\n");
      perror(__func__);
      exit(EXIT_FAILURE);
    }

    wrote = fwrite(buffer, 1, got, fp_dest);

    if (wrote != got)
    {
      remove(dest);
      fclose(fp_source);
      fclose(fp_dest);
      fprintf(stderr, "Error copying to file.\n");
      perror(__func__);
      exit(EXIT_FAILURE);
    }
  }
  fclose(fp_source);
  fclose(fp_dest);
}

// Synchronise older files, or create files, that match the newest version of that file.
void sync_files(int num_dir)
{
  // Iterate through keys of hashmap. i.e. the unique files.
  for (int key = 0; key < key_count; key++)
  {
    LIST *file;
    // Boolean mask of directories passed to program.
    bool *mask = calloc(num_dir, sizeof(bool));
    for (int dir = 0; dir < num_dir; dir++)
    {
      // If directory does not have the newest version of the file, or does not exist, set mask[dir] to false.
      mask[dir] = hashmap_find(hashmap_newest, keys[key], top_directories[dir]);
      if (mask[dir] == true)
      {
        // If directory does have the newest version, we keep track of the file.
        file = hashmap_return(hashmap_newest, keys[key], top_directories[dir]);
      }
    }

    // Path of the newest version of the file.
    char source[MAXPATHLEN];
    snprintf(source, MAXPATHLEN, "%s%s", file->top_level, file->path_from_top);
    char dest[MAXPATHLEN];

    // Iterate over each directory passed to program.
    for (int dir = 0; dir < num_dir; dir++)
    {
      // If directory does not have the newest version...
      if (mask[dir] == false)
      {
        // ...set dest to this directory.
        snprintf(dest, MAXPATHLEN, "%s%s", top_directories[dir], keys[key]);
        if (verbose)
        {
          char *dir_tomake = strdup(keys[key]);
          char *p = strrchr(dir_tomake, '/');
          // Check if there are any directories in path.
          if (p == NULL)
          {
            printf("No directory creation needed to copy file \"%s\" to \"%s\"\n",
                   source, top_directories[dir]);
          }
          else
          {
            p++;
            *p = '\0';
            printf("Directory \"%s\" will be created in \"%s\"\n", dir_tomake, top_directories[dir]);
            printf("File \"%s\" will be copied to \"%s%s\"\n", source, top_directories[dir], dir_tomake);
            free(dir_tomake);
          }
        }
        if (!no_sync)
        {
          // Only create directories and copy files over if -n is not provided.
          create_dirs(top_directories[dir], keys[key]);
          fcopy(source, dest);
        }
        // Set permissions of new files to 'old' newest file if -p is provided.
        if (same_permission)
        {
          time_t mtime = file->mod_time;
          if (verbose)
          {
            printf("Setting modification time of \"%s\" to %s", dest, ctime(&mtime));
          }
          // Set permissions
          chmod(dest, file->mode);

          // Set modification time.
          struct utimbuf ubuf;
          time_t ctime;
          time(&ctime);
          ubuf.modtime = mtime;
          ubuf.actime = ctime;
          utime(dest, &ubuf);
        }
      }
    }
    free(mask);
  }
  printf("\n");
}

// Find newest version of all files.
void find_difference(int num_dir)
{

  LIST *file;
  time_t newest_time = 0;
  hashmap_newest = new_hashmap();

  // For each unique file under any "top level" directory
  for (int key = 0; key < key_count; key++)
  {
    for (int pass = 0; pass < 2; pass++)
    {
      // For each "top level" directory.
      for (int dir = 0; dir < num_dir; dir++)
      {
        file = hashmap_return(hashmap_main, keys[key], top_directories[dir]);
        if (file == NULL)
        {
          continue;
        }
        // First pass simply finds the modification time of the newest file.
        else if (pass == 0)
        {
          if (difftime(file->mod_time, newest_time) >= 0)
          {
            newest_time = file->mod_time;
          }
        }
        // Second pass stores any files that match the newest modification time.
        else if (pass == 1)
        {
          if (file->mod_time == newest_time)
          {
            hashmap_add(hashmap_newest, file->path_from_top, file->top_level, file->mod_time, file->mode);
            if (verbose)
            {
              printf("The newest version of \"%s\" is in directory \"%s\"\n", file->path_from_top, file->top_level);
            }
          }
        }
      }
    }
    // Reset newest_time.
    newest_time = 0;
  }
  printf("\n");
}
