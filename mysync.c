#include "mysync.h"

// TODO: Maybe move #include statements that only get used in one file to that file.
// TODO: Check that functions only used in one file are only declared in that file.
// TODO: free() dynamic memory as soon as they are finished.
// TODO: put verbose condition before print statements.

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

  if (verbose)
  {
    print_hashmap(hashmap_main);
  }

  find_difference(argc);

  if (!no_sync)
  {
    sync_files(argc);
  }

  return 0;
}
