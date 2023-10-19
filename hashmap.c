#include "mysync.h"

// Maybe change functions to accept LIST not just individual fields

// Early idea on how to implement -r
// a file in dir1/dir2/file.txt would be stored in data struct with parent directory dir1/dir2
// As opposed to just dir1
// If we are syncing dir1 and dir3, with dir1 having dir2 inside, we can check for the existence of dir2 in dir3, and if not,
// Create dir2 in dir3. etc. Check sample solution for if dir3 has file.txt, but not dir2.

// List functions are not declared in header, as they are only used in this file.
// Given pathname, check if file exists in given "top-level" directory.
bool list_find(LIST *list, char *top_level, char *path_from_top)
{
  while (list != NULL)
  {
    if (strcmp(list->top_level, top_level) == 0)
    {
      if (strcmp(list->path_from_top, path_from_top) == 0)
      {
        return true;
      }
    }
    list = list->next;
  }
  return false;
}

LIST *new_list_item(char *top_level, char *path_from_top, int mtime)
{
  LIST *new = calloc(1, sizeof(LIST));
  CHECK_ALLOC(new);

  new->top_level = strdup(top_level);
  CHECK_ALLOC(new->top_level);

  new->path_from_top = strdup(path_from_top);
  CHECK_ALLOC(new->path_from_top);

  new->mod_time = mtime;
  new->next = NULL;
  return new;
}

// top_level is directory passed to mysync in command line
// path_from_top if the path from the top_level to the file.
LIST *list_add(LIST *list, char *path_from_top, char *top_level, int mtime)
{
  if (list_find(list, top_level, path_from_top))
  {
    return list;
  }
  else
  {
    LIST *new = new_list_item(top_level, path_from_top, mtime);
    new->next = list;
    return new;
  }
}

void print_list(LIST *list)
{
  if (list != NULL)
  {
    while (list != NULL)
    {
      time_t mtime = list->mod_time;
      printf("File \"%s\" exists in directory: %s, (%s)\n", list->path_from_top, list->top_level, ctime(&mtime));
      list = list->next;
    }
  }
}

// Hashmap function definitions.
uint32_t hash_string(char *string)
{
  uint32_t hash = 0;

  while (*string != '\0')
  {
    hash = hash * 33 + *string;
    ++string;
  }
  return hash;
}

HASHMAP *new_hashmap(void)
{
  HASHMAP *new = calloc(hashmap_size, sizeof(LIST *));
  CHECK_ALLOC(new);
  return new;
}

bool hashmap_add(HASHMAP *hashmap, char *path_from_top, char *top_level, int mtime)
{
  uint32_t h = hash_string(path_from_top) % hashmap_size;
  char *current;
  if (hashmap[h] == NULL)
  {
    current = "";
  }
  else
  {
    current = strdup(hashmap[h]->path_from_top);
    CHECK_ALLOC(current);
  }

  hashmap[h] = list_add(hashmap[h], path_from_top, top_level, mtime);

  if (strcmp(hashmap[h]->path_from_top, current) == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool hashmap_find(HASHMAP *hashmap, char *path_from_top, char *top_level)
{
  uint32_t h = hash_string(path_from_top) % hashmap_size;
  return list_find(hashmap[h], top_level, path_from_top);
}

LIST *hashmap_return(HASHMAP *hashmap, char *path_from_top, char *top_level)
{
  uint32_t h = hash_string(path_from_top) % hashmap_size;
  LIST *list = hashmap[h];

  while (list != NULL)
  {
    if (strcmp(list->top_level, top_level) == 0)
    {
      if (strcmp(list->path_from_top, path_from_top) == 0)
      {
        return list;
      }
    }
    list = list->next;
  }
  return NULL;
}

void print_hashmap(HASHMAP *hashmap)
{
  for (int i = 0; i < hashmap_size; i++)
  {
    if (hashmap[i] == NULL)
    {
      continue;
    }
    else
    {
      print_list(hashmap[i]);
    }
  }
}
