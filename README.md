# Unified description of checker parameters for Clang Static Analyser

**Note: this is the README for the unified description feature, staarting at commit 673f0a5. The original Clang README is at README-clang.md**

Software often contains defects that are hard to identify, because in most cases they do not cause the execution to fail. Static analysis is one of the ways to fix these defects. As one of the most popular C/C++ compilers, Clang has a built-in static analyzer. However, it has a major drawback, forcing developers to manage different ways of configuring it, in order to fully use it. This work proposes a method to overcome this problem.

One of the simplest way to provide information to the Clang Static Analyser checkers is to put it in the source code as calls to special functions that are processed during static analysis and ignored at other build configurations. Here is an example of a memory allocation function *some_alloc*, annotated with such special functions. This definition should be present in source files used only for static analysis builds, while the actual function body would be in the library linked during actual builds.
```
void* some_alloc(int cnt, int size, bool isCalloc)
{
  void* ptr;
  if (isCalloc)
    ptr = sf_allocate_with_size(size * cnt);
  else
    ptr = sf_allocate_with_size(cnt);
  return ptr;
}
```
Assuming we know from checker documentation that *sf_allocate_with_size(size)* means function will allocate *size* bytes of memory, we can see that this function will use malloc- or calloc-like allocation, depending on *isCalloc* parameter. This is easy for developer to keep track of, and allows to provide conditional annotations as well.

This project presents two ways of implementing such feature, namely description converter and description manager.

## Description converter
Description converter converts the unified description to a format supported by the existing Clang checker. To use it, you need to implement a subclass of *DescriptionConverter*, defining methods *processFunction* and *outputConfiguration*. The former will be called when the checker starts processing a source code function, while the latter is called at the end of analysis to output the result. See clang/lib/StaticAnalyzer/Checkers/DescriptionConverter/DescriptionConverters/TaintCheckerDescriptionConverter.cpp for implementation example.

## Description manager
Description manager provides an interface for checkers to obtain the descriptions for a given function. To use it, you need to register custom description functions by calling *DescriptionManage::addSFunction* method with special function name as argument. After that you can invoke *getParams* in checker methods to get a list of arguments of a given special function. See clang/lib/StaticAnalyzer/Checkers/ArrayBoundCheckerV2.cpp for implementation example.