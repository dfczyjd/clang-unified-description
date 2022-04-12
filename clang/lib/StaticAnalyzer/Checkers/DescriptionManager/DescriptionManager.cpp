#include "DescriptionManager.h"

SFunctionVisitor *DescriptionManager::Walker;
llvm::StringSet<> DescriptionManager::RegisteredFunctions;
std::map<const FunctionDecl *, llvm::StringMap<llvm::SmallVector<ParamVector>>>
    DescriptionManager::SFParams;

void DescriptionManager::SetWalker(SFunctionVisitor *Walker) {
  DescriptionManager::Walker = Walker;
}

llvm::SmallVector<ParamVector>
DescriptionManager::GetParams(llvm::StringRef SFName,
                              const FunctionDecl *Function) {
  llvm::StringMap<llvm::SmallVector<ParamVector>> sFunctions;
  if (!Function->hasBody()) {
    // Function is from library, there are no SFs in its body
    return llvm::SmallVector<ParamVector>();
  }
  if (SFParams.find(Function) == SFParams.end()) {
    Walker->Visit(Function->getBody());
    sFunctions = Walker->GetFoundSFs();
    SFParams[Function] = sFunctions;
  } else {
    sFunctions = SFParams[Function];
  }
  if (sFunctions.find(SFName) == sFunctions.end())
    return llvm::SmallVector<ParamVector>();
  return sFunctions[SFName];
}

void DescriptionManager::ParseSFunctions(SFunctionVisitor &walker,
                                         const Stmt *stmt) {
  walker.Visit(stmt);
}

int DescriptionManager::LookForArgument(const clang::FunctionDecl *function,
                                          llvm::StringRef argName) {
  int i = 0;
  for (auto it = function->param_begin(), end = function->param_end();
       it != end; ++it, ++i) {
    if ((*it)->getName() == argName)
      return i;
  }
  return -1;
}
