#include "mysync.h"

// Given relative path, check if file exists in given "top-level" directory.
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

// Create a new list item
LIST *new_list_item(char *top_level, char *path_from_top, time_t mtime, mode_t mode)
{
  LIST *new = calloc(1, sizeof(LIST));
  CHECK_ALLOC(new);

  new->top_level = strdup(top_level);
  CHECK_ALLOC(new->top_level);

  new->path_from_top = strdup(path_from_top);
  CHECK_ALLOC(new->path_from_top);

  new->mod_time = mtime;
  new->mode = mode;
  new->next = NULL;

  return new;
}

// Add item to list. Only adds item if item does not already exist.
LIST *list_add(LIST *list, char *path_from_top, char *top_level, time_t mtime, mode_t mode)
{
  if (list_find(list, top_level, path_from_top))
  {
    return list;
  }
  else
  {
    LIST *new = new_list_item(top_level, path_from_top, mtime, mode);
    new->next = list;
    return new;
  }
}

// Print all items in list.
void print_list(LIST *list)
{
  if (list != NULL)
  {
    while (list != NULL)
    {
      time_t mtime = list->mod_time;
      printf("File \"%s\" exists in directory: %s, last modified: %s", list->path_from_top, list->top_level, ctime(&mtime));
      list = list->next;
    }
  }
}

// Create hash from a string.
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

// Create a new hashmap.
HASHMAP *new_hashmap(void)
{
  HASHMAP *new = calloc(hashmap_size, sizeof(LIST *));
  CHECK_ALLOC(new);
  return new;
}

// Adds item to hashmap. Returns true if there is not already an item for the given key.
bool hashmap_add(HASHMAP *hashmap, char *path_from_top, char *top_level, time_t mtime, mode_t mode)
{
  uint32_t h = hash_string(path_from_top) % hashmap_size;
  char *current;

  if (hashmap[h] == NULL)
  {
    current = "";
  }
  else
  {
    // If there is an item under the hash, store the key.
    current = strdup(hashmap[h]->path_from_top);
    CHECK_ALLOC(current);
  }

  hashmap[h] = list_add(hashmap[h], path_from_top, top_level, mtime, mode);

  // The comp is just in case of hashmap collisions.
  // Store the key that we just stored, with the "current" key.
  if (strcmp(hashmap[h]->path_from_top, current) == 0)
  {
    return false;
  }
  else
  {
    // If the item we just stored if the first item for that particular key, return true.
    return true;
  }
}

// Return true if key-value pair exists in hashmap.
bool hashmap_find(HASHMAP *hashmap, char *path_from_top, char *top_level)
{
  uint32_t h = hash_string(path_from_top) % hashmap_size;
  return list_find(hashmap[h], top_level, path_from_top);
}

// Return item that corresponds to key-value pair in given hashmap.
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

// Print entire hashmap.
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
  printf("\n");
}
