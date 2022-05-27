#include "descriptions.h"
#include <stdlib.h>
#include <stdio.h>

void consume(int arg1)
{
  sf_sink(arg1);
	return;
}

void returnTaint(int *arg)
{
  sf_propagation_src(arg);
}

int produce()
{
  sf_propagation_return();
  return fgetc(0);
}

void filter(int& val)
{
  sf_filter(val);
  return;
}

int main() 
{
  int val = produce();
  returnTaint(&val);
  consume(val);   // taintChecker: tainted data reached sink
  filter(val);
  consume(val);   // taintChecker: no warning
  return 0;
}
