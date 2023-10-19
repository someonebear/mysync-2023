#include "mysync.h"

bool all_files = false;
bool no_sync = false;
bool same_permission = false;
bool recursive = false;
bool verbose = false;

int num_files = 0;
int hashmap_size = 0;

HASHMAP *hashmap_main = NULL;
HASHMAP *hashmap_newest = NULL;

// For some reason char *[] doesn't work, while char ** does even though they are the same.
char **top_directories = NULL;
char **keys = NULL;
int key_count = 0;