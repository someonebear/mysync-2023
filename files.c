#include "mysync.h"

// Create all missing directories in path.
void create_dirs(char *created, char *path)
{
  // maybe pass top_level and path_from_top to this function, then we can create next level down with
  //  %s%s, top_level, next_level,
  //  with next_level being like %s/%*s sscanf or something
  char to_create[MAXPATHLEN];
  char exists[MAXPATHLEN];

  strcpy(to_create, path);
  strcpy(exists, created);

  char *p = strchr(to_create, '/');
  // base case, just a file, no directories.
  if (p == NULL)
  {
    return;
  }
  // recursive step, make first directory in "to_create", and remove from "to_create", add to "exists"
  else
  {
    char *first_dir = strdup(to_create);
    char *p2 = strchr(first_dir, '/');
    p2++;
    *p2 = '\0';

    snprintf(exists, MAXPATHLEN, "%s%s", exists, first_dir);
    if (mkdir(exists) != 0)
    {
      if (errno != EEXIST)
      {
        fprintf(stderr, "Could not create directory: %s\n", exists);
        perror(__func__);
        exit(EXIT_FAILURE);
      }
    }

    p++;
    strcpy(to_create, p);
    create_dirs(exists, to_create);
  }
}
// Go along entire path, if directory exists, move on, if it doesnt, create it.

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
      fprintf(stderr, "Error copying to file.\n");
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

void sync_files(int num_dir)
{
  for (int key = 0; key < key_count; key++)
  {
    LIST *file;
    bool *mask = calloc(num_dir, sizeof(bool));
    for (int dir = 0; dir < num_dir; dir++)
    {
      mask[dir] = hashmap_find(hashmap_newest, keys[key], top_directories[dir]);
      if (mask[dir] == true)
      {
        file = hashmap_return(hashmap_newest, keys[key], top_directories[dir]);
      }
    }

    char source[MAXPATHLEN];
    snprintf(source, MAXPATHLEN, "%s%s", file->top_level, file->path_from_top);
    char dest[MAXPATHLEN];

    for (int dir = 0; dir < num_dir; dir++)
    {
      if (mask[dir] == false)
      {
        snprintf(dest, MAXPATHLEN, "%s%s", top_directories[dir], keys[key]);
        if (verbose)
        {
          char *dir_tomake = strdup(keys[key]);
          char *p = strrchr(dir_tomake, '/');
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
          create_dirs(top_directories[dir], keys[key]);
          fcopy(source, dest);
        }
      }
    }
    free(mask);
  }
}
// Create bitmask for each key, indicating which top directory has up-to-date, and which has older/doesnt exist
// Call create_dir to create directories on path if file doesn't exist

void find_difference(int num_dir)
{
  // For each key, and top-level, hashmap_find.
  // If it exists, compare to newest version. Newest versions get added to hashmap_newest.
  // If -n is called, i guess call sync-files with 'n' mode
  // If not, call sync-files with some other mode.
  LIST *file;
  // LIST **newest_file = calloc(num_dir, sizeof(LIST *));
  time_t newest_time = 0;

  hashmap_newest = new_hashmap();

  for (int key = 0; key < key_count; key++)
  {
    for (int pass = 0; pass < 2; pass++)
    {
      for (int dir = 0; dir < num_dir; dir++)
      {
        // its actually giving seg fault here.
        // Num_files is actually counting test3.txt twice.
        // I wasn't incrementing key_count.
        file = hashmap_return(hashmap_main, keys[key], top_directories[dir]);
        if (file == NULL)
        {
          continue;
        }
        else if (pass == 0)
        {
          if (difftime(file->mod_time, newest_time) >= 0)
          {
            // This is setting each file newer than the last in the array.
            // Must change to only set the newest file for all of them.
            newest_time = file->mod_time;
          }
        }
        else if (pass == 1)
        {
          if (file->mod_time == newest_time)
          {
            hashmap_add(hashmap_newest, file->path_from_top, file->top_level, file->mod_time);
            if (verbose)
            {
              printf("The newest version of \"%s\" is in directory \"%s\"\n", file->path_from_top, file->top_level);
            }
          }
          // Add to newest_file
        }
      }
    }
    newest_time = 0;
  }
}
