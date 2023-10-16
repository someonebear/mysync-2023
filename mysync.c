#include "mysync.h"

void usage(void)
{
  fprintf(stderr,
          "usage: mysync [-a] [-i pattern] [-n] [-o pattern] [-p] [-r] [-v] directory1 directory2 [directory3 ...]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  // Not checking flags for now. Just that there are at least 2 dirs.
  int opt;
  opterr = 0;

  while ((opt = getopt(argc, argv, OPTLIST)) != -1)
  {
    switch (opt)
    {
    case 'a':
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

  argc -= optind;
  argv += optind;
  if (argc < 2)
  {
    fprintf(stderr, "Two or more directories are required to sync.\n");
    usage();
  }

  return 0;
}
