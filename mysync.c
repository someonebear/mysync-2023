#include "mysync.h"

int main(int argc, char *argv[])
{
  // Read and store options.
  validate_opt(argc, argv);

  // Setting argc to number of non option arguments.
  // Setting argv to first non option argument.
  argc -= optind;
  argv += optind;

  if (argc < 2)
  {
    fprintf(stderr, "Two or more directories are required to sync.\n");
    usage();
  }

  // Expand tilde if needed. Store directory names.
  process_dir(argc, argv);

  // Read contents of directories.
  read_dir(argc);

  if (verbose)
  {
    print_hashmap(hashmap_main);
  }

  // Find differences between directories passed to program.
  find_difference(argc);

  // Actual copying of files.
  sync_files(argc);

  if (verbose)
  {
    print_hashmap(hashmap_main);
  }

  return 0;
}
