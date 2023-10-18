#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdint.h>

// From Workshop 8
#define CHECK_ALLOC(p)  \
  if (p == NULL)        \
  {                     \
    perror(__func__);   \
    exit(EXIT_FAILURE); \
  }

// First colon makes getopt() return ':' if option arg is missing.
#define OPTLIST ":ai:no:prv"

// Variables to hold flag values
extern bool all_files;
extern char *ignore;
extern bool no_sync;
extern char *only_sync;
extern bool same_permission;
extern bool recursive;
extern bool verbose;

// Hashmap declarations
typedef struct list_item
{
  // Top-level directory that this item is under.
  char *top_level;
  time_t mod_time;
  // Just in case of collisions. Full path minus top-level, i.e. the key
  char *path_from_top;
  struct list_item *next;
} LIST;

typedef LIST *HASHMAP;

// Count of number of files, for hashmap size
extern int num_files;
extern int hashmap_size;

extern HASHMAP *hashmap;

extern HASHMAP *new_hashmap(void);

extern void hashmap_add(HASHMAP *, char *, char *, int);

extern bool hashmap_find(HASHMAP *, char *, char *, int);

extern void print_hashmap(HASHMAP *);

// Global array for names of arrays passed to program.
extern char *top_directories[];

// Global functions
extern void usage(void);
extern void validate_opt(int, char *[]);
extern void read_dir(int, char *[]);

extern void find_difference(void);
