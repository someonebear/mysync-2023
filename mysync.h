#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>

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

// Global functions
extern void usage(void);
extern void validate_opt(int, char *[]);
extern void read_dir(int, char *[]);

// Global data structures
extern DIR *directories[];