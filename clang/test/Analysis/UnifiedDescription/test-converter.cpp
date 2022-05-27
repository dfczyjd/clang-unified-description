#include "descriptions.h"
#include <stdlib.h>
#include <stdio.h>

int returnTaint(int arg1, int arg2)
{
  sf_propagation_src(arg1);
  sf_propagation_dst(arg2);
  return arg2;
}

void consume(int arg1, int arg2)
{
  sf_sink(arg1, arg2);
	return;
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

namespace nmsp {
  namespace inner {
int varargs_propagation(int arg1, int arg2, int params...) {
  sf_propagation_src(arg1);
  sf_propagation_return();
  sf_propagation_variadic(SF_TaintCheckerVarType::Src, 3);
  return 0;
}
  }
}

int main() 
{
  return 0;
}
