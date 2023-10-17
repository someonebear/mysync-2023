#include "mysync.h"

int main(int argc, char *argv[])
{
  validate_opt(argc, argv);

  argc -= optind;
  argv += optind;
  if (argc < 2)
  {
    fprintf(stderr, "Two or more directories are required to sync.\n");
    usage();
  }

  DIR *dirp;
  struct dirent *dp;
  while (argc > 0)
  {
    dirp = opendir(*argv);
    if (dirp == NULL)
    {
      fprintf(stderr, "Directory \"%s\" could not be opened.\n", *argv);
      perror("Error: ");
      exit(EXIT_FAILURE);
    }
    printf("Opened: %s\n", *argv);

    while ((dp = readdir(dirp)) != NULL)
    {
      printf("Opened: %s\n", dp->d_name);
    }
    argc--;
    argv++;
    closedir(dirp);
  }

  return 0;
}
