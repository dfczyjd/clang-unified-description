#include "sf_functions.h"
#include <stdlib.h>
#include <string.h>

void *malloc(size_t size)
{
  void* ptr;
  ptr = sf_allocate_with_size(size, SF_AllocationFamily::AF_Malloc);
  return ptr;
}

char *strdup(char *s)
{
  char* result;
  int size = strlen(s);
  result = (char*)sf_allocate_with_size(size, SF_AllocationFamily::AF_Malloc);
  return result;
}