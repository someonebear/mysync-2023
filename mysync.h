#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

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
  char *string;
  struct list_item *next;
} LIST;

typedef LIST *hashmap;

extern hashmap *new_hashmap(void);

extern void hashmap_add(hashmap *, char *);

extern bool hashmap_find(hashmap *, char *);

// Global functions
extern void usage(void);
extern void validate_opt(int, char *[]);
extern void read_dir(int, char *[]);
