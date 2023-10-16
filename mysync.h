#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

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