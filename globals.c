#include "mysync.h"

bool all_files = false;
bool no_sync = false;
bool same_permission = false;
bool recursive = false;
bool verbose = false;

int num_files = 0;
int hashmap_size = 0;
HASHMAP *hashmap = NULL;