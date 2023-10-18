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

  read_dir(argc, argv);

  print_hashmap(hashmap);

  return 0;
}
