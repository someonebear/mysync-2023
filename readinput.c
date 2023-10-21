#include "mysync.h"

void usage(void)
{
  fprintf(stderr,
          "usage: mysync [-a] [-i pattern] [-n] [-o pattern] [-p] [-r] [-v] directory1 directory2 [directory3 ...]\n");
  exit(EXIT_FAILURE);
}

void validate_opt(int argc, char *argv[])
{
  int opt;
  opterr = 0;

  while ((opt = getopt(argc, argv, OPTLIST)) != -1)
  {
    switch (opt)
    {
    case 'a':
      all_files = true;
      break;
    case 'i':
      break;
    case 'n':
      no_sync = true;
      verbose = true;
      break;
    case 'o':
      break;
    case 'p':
      same_permission = true;
      break;
    case 'r':
      recursive = true;
      break;
    case 'v':
      verbose = true;
      break;
    case '?':
      fprintf(stderr, "Invalid input: \"-%c\" is not a valid option.\n", optopt);
      usage();
    default:
      // If opt == ':'
      fprintf(stderr, "Option -%c requires pattern.\n", optopt);
      usage();
    }
  }
}

// All directory paths will have trailing '/', and no '/' at the beginning when stored.
// They can then be pieced together in format strings easier
char *add_slash(char *string)
{
  int length = strlen(string);
  if (string[length - 1] == '/')
  {
    return string;
  }
  else
  {
    sprintf(string, "%s/", string);
    return string;
  }
}

// Either 'c' mode for counting, or 's' mode for storing.
void find_files(DIR *dirp, char *top_level, char *path_from_top, char mode)
{
  struct dirent *dp;
  char fullpath[MAXPATHLEN];
  char rel_path[MAXPATHLEN];

  while ((dp = readdir(dirp)) != NULL)
  {
    struct stat stat_buffer;
    sprintf(fullpath, "%s%s%s", top_level, path_from_top, dp->d_name);
    if (stat(fullpath, &stat_buffer) != 0)
    {
      fprintf(stderr, "File stats for %s could not be found.\n", dp->d_name);
      perror("Error: ");
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(stat_buffer.st_mode) && recursive)
    {
      if (dp->d_name[0] == '.')
      {
        continue;
      }
      sprintf(fullpath, "%s%s%s", top_level, path_from_top, dp->d_name);
      DIR *recursive_dirp = opendir(fullpath);
      CHECK_ALLOC(recursive_dirp);
      // this should be path from top, and name of directory, so a new path from top for the next level.
      sprintf(rel_path, "%s%s/", path_from_top, dp->d_name);
      find_files(recursive_dirp, top_level, rel_path, mode);
    }

    else if (S_ISREG(stat_buffer.st_mode))
    {
      if (dp->d_name[0] == '.' && !all_files)
      {
        continue;
      }
      if (mode == 's')
      {
        if (verbose)
        {
          printf("File found: %s\n", dp->d_name);
        }
        sprintf(rel_path, "%s%s", path_from_top, dp->d_name);
        if (hashmap_add(hashmap_main, rel_path, top_level, stat_buffer.st_mtime, stat_buffer.st_mode))
        {
          *(keys + key_count) = strdup(rel_path);
          key_count++;
        }
      }
      else if (mode == 'c')
      {
        num_files += 1;
      }
    }
  }
}

void process_dir(int num_dir, char *dirs[])
{
  top_directories = calloc(num_dir, sizeof(char *));
  CHECK_ALLOC(top_directories);

  for (int i = 0; i < num_dir; i++)
  {
    char *buf = strdup(*dirs);
    char *p = strchr(buf, '~');
    if (p == NULL)
    {
      *(top_directories + i) = strdup(buf);
      CHECK_ALLOC(*(top_directories + i));
    }
    else
    {
      char *path = calloc(MAXPATHLEN, sizeof(char));
      CHECK_ALLOC(path);
      char *before;
      char *after;
      *p = '\0';
      p++;
      before = buf;
      after = p;
      snprintf(path, MAXPATHLEN, "%s%s%s", before, getenv("HOME"), after);
      *(top_directories + i) = strdup(path);
      CHECK_ALLOC(*(top_directories + i));
      free(path);
    }
    free(buf);
    dirs++;
  }
}

void read_dir(int num_dir)
{
  DIR *directories[num_dir];

  for (int i = 0; i < num_dir; i++)
  {
    *(top_directories + i) = add_slash(*(top_directories + i));

    // Opening each directory passed to program.
    directories[i] = opendir(*(top_directories + i));
    CHECK_ALLOC(*(directories + i));
    if (verbose)
    {
      printf("Opened: %s\n", *(top_directories + i));
    }
  }

  for (int i = 0; i < num_dir; i++)
  {
    // Count how many files exist, for hashmap size.
    find_files(directories[i], *(top_directories + i), "", 'c');
    rewinddir(directories[i]);
  }
  if (num_files == 0)
  {
    if (verbose)
    {
      printf("No files found.\n");
    }
    return;
  }

  hashmap_size = (num_files * 2);
  hashmap_main = new_hashmap();
  keys = calloc(num_files, sizeof(char *));

  for (int i = 0; i < num_dir; i++)
  {
    // Store each file in hashmap.
    find_files(directories[i], *(top_directories + i), "", 's');
    closedir(directories[i]);
  }
}