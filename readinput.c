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
      break;
    case 'o':
      break;
    case 'p':
      break;
    case 'r':
      recursive = true;
      break;
    case 'v':
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

void count_files(DIR *dirp, char *dir_name)
{
  struct dirent *dp;
  char fullpath[MAXPATHLEN];

  while ((dp = readdir(dirp)) != NULL)
  {
    struct stat stat_buffer;
    sprintf(fullpath, "%s/%s", dir_name, dp->d_name);
    if (stat(fullpath, &stat_buffer) != 0)
    {
      fprintf(stderr, "File stats for %s could not be found.\n", dp->d_name);
      perror("Error: ");
      exit(EXIT_FAILURE);
    }
    else if (S_ISREG(stat_buffer.st_mode))
    {
      if (dp->d_name[0] == '.' && !all_files)
      {
        continue;
      }
      hashmap_size += 2;
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

void find_files(DIR *dirp, char *top_level, char *path_from_top)
{
  struct dirent *dp;
  char fullpath[MAXPATHLEN];
  char rel_path[MAXPATHLEN];
  while ((dp = readdir(dirp)) != NULL)
  {
    struct stat stat_buffer;
    // TODO
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
      // TODO this should be path from top, and name of directory, so a new path from top for the next level.
      sprintf(rel_path, "%s%s/", path_from_top, dp->d_name);
      find_files(recursive_dirp, top_level, rel_path);
    }

    else if (S_ISREG(stat_buffer.st_mode))
    {
      if (dp->d_name[0] == '.' && !all_files)
      {
        continue;
      }
      printf("File found: %s\n", dp->d_name);
      sprintf(rel_path, "%s%s", path_from_top, dp->d_name);
      hashmap_add(hashmap, rel_path, top_level, stat_buffer.st_mtime);
    }
  }
}

void read_dir(int num_dir, char *dirs[])
{
  DIR *directories[num_dir];

  for (int i = 0; i < num_dir; i++)
  {
    *dirs = add_slash(*dirs);
    directories[i] = opendir(*dirs);
    CHECK_ALLOC(directories[i]);
    printf("Opened: %s\n", *dirs);
    dirs++;
  }

  dirs -= num_dir;
  for (int i = 0; i < num_dir; i++)
  {
    count_files(directories[i], *dirs);
    rewinddir(directories[i]);
    dirs++;
  }

  hashmap = new_hashmap();

  dirs -= num_dir;
  for (int i = 0; i < num_dir; i++)
  {
    find_files(directories[i], *dirs, "");
    closedir(directories[i]);
    dirs++;
  }
}