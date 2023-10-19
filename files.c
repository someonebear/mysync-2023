#include "mysync.h"

// Create all missing directories in path.
void create_dirs(char *path);
// Go along entire path, if directory exists, move on, if it doesnt, create it.

void sync_files()
{
  return;
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

  HASHMAP *hashmap_newest = new_hashmap();

  for (int key = 0; key < key_count; key++)
  {
    for (int pass = 0; pass < 3; pass++)
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
