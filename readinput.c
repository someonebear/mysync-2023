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

void find_files(DIR *dirp, char *dir_name)
{
  struct dirent *dp;
  char fullpath[MAXPATHLEN];

  while ((dp = readdir(dirp)) != NULL)
  {
    struct stat stat_buffer;
    sprintf(fullpath, "%s/%s", dir_name, dp->d_name);
    if (stat(fullpath, &stat_buffer) != 0)
    {
      perror("Error: ");
      exit(EXIT_FAILURE);
    }
    else if (S_ISREG(stat_buffer.st_mode))
    {
      if (dp->d_name[0] == '.' && !all_files)
      {
        continue;
      }
      printf("File found: %s\n", dp->d_name);
    }
  }
}

void read_dir(int num_dir, char *dir[])
{
  DIR *directories[num_dir];

  for (int i = 0; i < num_dir; i++)
  {
    directories[i] = opendir(*dir);
    if (directories[i] == NULL)
    {
      fprintf(stderr, "Directory \"%s\" could not be opened.\n", *dir);
      perror("Error: ");
      exit(EXIT_FAILURE);
    }
    printf("Opened: %s\n", *dir);
    dir++;
  }

  dir -= num_dir;
  for (int i = 0; i < num_dir; i++)
  {
    find_files(directories[i], *dir);
    closedir(directories[i]);
    dir++;
  }
}