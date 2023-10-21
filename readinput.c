#include <regex.h>
#include <dirent.h>

#include "mysync.h"

static char **ignore_pattern = NULL;
static char **only_pattern;
static int nignore = 0;
static int nonly = 0;

// Print usage message.
void usage(void)
{
  fprintf(stderr,
          "usage: mysync [-a] [-i pattern] [-n] [-o pattern] [-p] [-r] [-v] directory1 directory2 [directory3 ...]\n");
  exit(EXIT_FAILURE);
}

// Validate and read options given to program.
void validate_opt(int argc, char *argv[])
{
  int opt;
  opterr = 0;

  while ((opt = getopt(argc, argv, OPTLIST)) != -1)
  {
    switch (opt)
    {
    case 'a':
      all_files = true;
      break;
    case 'i':
      // If first -i encountered, create array
      // If not, resize array to fit
      // Store pattern in array.
      if (nignore == 0)
      {
        nignore++;
        ignore_pattern = calloc(nignore, sizeof(char *));
      }
      else
      {
        nignore++;
        ignore_pattern = realloc(ignore_pattern, nignore * sizeof(char *));
      }
      CHECK_ALLOC(ignore_pattern);
      *(ignore_pattern + (nignore - 1)) = strdup(optarg);
      CHECK_ALLOC(*(ignore_pattern + (nignore - 1)));
      break;
    case 'n':
      no_sync = true;
      verbose = true;
      break;
    case 'o':
      if (nonly == 0)
      {
        nonly++;
        only_pattern = calloc(nonly, sizeof(char *));
      }
      else
      {
        nonly++;
        only_pattern = realloc(only_pattern, nonly * sizeof(char *));
      }
      CHECK_ALLOC(only_pattern);
      *(only_pattern + (nonly - 1)) = strdup(optarg);
      CHECK_ALLOC(*(only_pattern + (nonly - 1)));
      break;
    case 'p':
      same_permission = true;
      break;
    case 'r':
      recursive = true;
      break;
    case 'v':
      verbose = true;
      break;
    case '?':
      fprintf(stderr, "Invalid input: \"-%c\" is not a valid option.\n", optopt);
      usage();
    default:
      // If opt == ':'
      fprintf(stderr, "Option -%c requires pattern.\n", optopt);
      usage();
    }
  }
}

// Returns true if filename matches any ignore patterns.
bool match_ignore(char *filename)
{
  for (int i = 0; i < nignore; i++)
  {
    regex_t r;
    char *regex = glob2regex(*(ignore_pattern + i));
    int status;

    status = regcomp(&r, regex, 0);
    if (status != 0)
    {
      fprintf(stderr, "Could not compile regex.\n");
      exit(EXIT_FAILURE);
    }

    status = regexec(&r, filename, 0, NULL, 0);
    if (status == 0)
    {
      if (verbose)
      {
        printf("File \"%s\" matches -i pattern \"%s\"\n", filename, *(ignore_pattern + i));
        printf("File will be ignored.\n");
        return true;
      }
    }
    else if (status == REG_NOMATCH)
    {
      continue;
    }
    else
    {
      perror(__func__);
      exit(EXIT_FAILURE);
    }
  }
  if (verbose)
  {
    printf("File \"%s\" matches no -i patterns.\n");
  }
  return false;
}

// Returns true if filename matches any -o patterns.
bool match_only(char *filename)
{
  for (int i = 0; i < nonly; i++)
  {
    regex_t r;
    char *regex = glob2regex(*(only_pattern + i));
    int status;

    status = regcomp(&r, regex, 0);
    if (status != 0)
    {
      fprintf(stderr, "Could not compile regex.\n");
      exit(EXIT_FAILURE);
    }

    status = regexec(&r, filename, 0, NULL, 0);
    if (status == 0)
    {
      if (verbose)
      {
        printf("File \"%s\" matches -o pattern \"%s\"\n", filename, *(only_pattern + i));
        printf("File will be added.\n");
      }
      return true;
    }
    else if (status == REG_NOMATCH)
    {
      continue;
    }
    else
    {
      perror(__func__);
      exit(EXIT_FAILURE);
    }
  }
  if (verbose)
  {
    printf("File \"%s\" matches no -o patterns.\n");
  }
  return false;
}

// All directory paths will have trailing '/', and no '/' at the beginning when stored.
// They can then be pieced together in format strings easier
// realloc is used to prevent buffer overflow.
char *add_slash(char *string)
{
  int length = strlen(string);
  if (string[length - 1] == '/')
  {
    return string;
  }
  else
  {
    string = realloc(string, length + 1);
    CHECK_ALLOC(string);
    sprintf(string, "%s/", string);
    return string;
  }
}

// Either 'c' mode for counting, or 's' mode for storing.
void find_files(DIR *dirp, char *top_level, char *path_from_top, char mode)
{
  struct dirent *dp;
  char fullpath[MAXPATHLEN];
  char rel_path[MAXPATHLEN];

  while ((dp = readdir(dirp)) != NULL)
  {
    struct stat stat_buffer;
    // Full path of current file.
    sprintf(fullpath, "%s%s%s", top_level, path_from_top, dp->d_name);
    if (stat(fullpath, &stat_buffer) != 0)
    {
      fprintf(stderr, "File stats for %s could not be found.\n", dp->d_name);
      perror("Error: ");
      exit(EXIT_FAILURE);
    }
    if (S_ISDIR(stat_buffer.st_mode) && recursive)
    {
      if (dp->d_name[0] == '.')
      {
        continue;
      }
      DIR *recursive_dirp = opendir(fullpath);
      CHECK_ALLOC(recursive_dirp);
      // The new "path_from_top" variable for next recursive call.
      sprintf(rel_path, "%s%s/", path_from_top, dp->d_name);
      // Recursively call function on directories.
      find_files(recursive_dirp, top_level, rel_path, mode);
    }

    else if (S_ISREG(stat_buffer.st_mode))
    {
      if (dp->d_name[0] == '.' && !all_files)
      {
        // Skip hidden files if -a is not used.
        continue;
      }
      else if (nignore > 0 && match_ignore(dp->d_name))
      {
        // Skip file if file matches any -i patterns.
        continue;
      }
      else if (nonly > 0 && !match_only(dp->d_name))
      {
        // Skip file if file does not match any -o patterns.
        continue;
      }

      if (mode == 's')
      {
        if (verbose)
        {
          printf("File found: %s\n", dp->d_name);
        }
        // rel_path is essentially the key of the dictionary.
        sprintf(rel_path, "%s%s", path_from_top, dp->d_name);
        if (hashmap_add(hashmap_main, rel_path, top_level, stat_buffer.st_mtime, stat_buffer.st_mode))
        {
          // Only increase key_count and store key if the key had not been seen before.
          *(keys + key_count) = strdup(rel_path);
          key_count++;
        }
      }
      else if (mode == 'c')
      {
        // Mode for counting number of files in directories.
        num_files += 1;
      }
    }
  }
  // Close current directory when done.
  closedir(dirp);
  printf("\n");
}

// In case of absolute paths with tildes. Expand all paths to something we can open.
void process_dir(int num_dir, char *dirs[])
{
  // To store all directories given to program.
  top_directories = calloc(num_dir, sizeof(char *));
  CHECK_ALLOC(top_directories);

  for (int i = 0; i < num_dir; i++)
  {
    char *buf = strdup(*dirs);
    // Tilde expansion. Find occurrence of ~
    char *p = strchr(buf, '~');
    if (p == NULL)
    {
      *(top_directories + i) = strdup(buf);
      CHECK_ALLOC(*(top_directories + i));
    }
    // If ~ found, expand to current HOME directory.
    else
    {
      char *path = calloc(MAXPATHLEN, sizeof(char));
      CHECK_ALLOC(path);
      char *before;
      char *after;
      // Splitting buf into two parts.
      *p = '\0';
      p++;
      before = buf;
      after = p;

      // Combining back into a full path.
      snprintf(path, MAXPATHLEN, "%s%s%s", before, getenv("HOME"), after);
      *(top_directories + i) = strdup(path);
      CHECK_ALLOC(*(top_directories + i));
      free(path);
    }
    free(buf);
    dirs++;
  }
}

// Read contents of directories. Open directories, count files, store files.
void read_dir(int num_dir)
{
  // Array for all open directories.
  DIR *directories[num_dir];

  for (int i = 0; i < num_dir; i++)
  {
    // Adding trailing slash, for consistent formatting.
    *(top_directories + i) = add_slash(*(top_directories + i));

    // Opening each directory passed to program.
    directories[i] = opendir(*(top_directories + i));
    CHECK_ALLOC(*(directories + i));
    if (verbose)
    {
      printf("Opened: %s\n", *(top_directories + i));
    }
  }
  printf("\n");

  for (int i = 0; i < num_dir; i++)
  {
    // Count how many files exist, for hashmap size.
    find_files(directories[i], *(top_directories + i), "", 'c');
    rewinddir(directories[i]);
  }

  if (num_files == 0)
  {
    if (verbose)
    {
      printf("No files found.\n");
    }
    return;
  }

  // Hashmap size is twice the size of number of files, to decrease hashmap load factor.
  hashmap_size = (num_files * 2);
  hashmap_main = new_hashmap();

  // Initialise array to store keys of hashmap.
  keys = calloc(num_files, sizeof(char *));

  for (int i = 0; i < num_dir; i++)
  {
    // Store each file in hashmap.
    find_files(directories[i], *(top_directories + i), "", 's');
  }
}