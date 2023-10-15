#include "mysync.h"

int main(int argc, char *argv[])
{
  // Not checking flags for now. Just that there are at least 2 dirs.
  if (argc < 3)
  {
    fprintf(stderr, "Two directories required.\n");
    exit(EXIT_FAILURE);
  }
  return 0;
}
