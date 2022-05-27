#include "descriptions.h"
#include <stdlib.h>
#include <string.h>

void* av_malloc(size_t);
void av_free(void*);

int main()
{
  int *a = (int*)av_malloc(12);
  a[4] = 0; // arrayBound: index out of bounds
  av_free(a);
  return 0;
}