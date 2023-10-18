#include "mysync.h"

// File will include hashmap functions, create, add, index ,etc.

// Hashmap will store directory containing file as key, and custom data struc as value.
// Data struct will contain file name, mod time, parent directory (maybe to prevent collisions).

// Early idea on how to implement -r
// a file in dir1/dir2/file.txt would be stored in data struct with parent directory dir1/dir2
// As opposed to just dir1
// If we are syncing dir1 and dir3, with dir1 having dir2 inside, we can check for the existence of dir2 in dir3, and if not,
// Create dir2 in dir3. etc. Check sample solution for if dir3 has file.txt, but not dir2.

// List functions are not declared in header, as they are only used in this file.
LIST *new_list(void);

LIST *list_add(LIST *list, char *string);

bool list_find(LIST *list, char *wanted);

void print_list(LIST *list);