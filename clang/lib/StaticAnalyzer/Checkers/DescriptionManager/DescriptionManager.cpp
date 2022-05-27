#include "DescriptionManager.h"

SFunctionVisitor *DescriptionManager::Walker;
llvm::StringSet<> DescriptionManager::RegisteredFunctions;
std::map<const FunctionDecl *, llvm::StringMap<llvm::SmallVector<ParamVector>>>
    DescriptionManager::SFParams;

void DescriptionManager::setWalker(SFunctionVisitor *Walker) {
  DescriptionManager::Walker = Walker;
}

ParamVector DescriptionManager::processCallExpr(const clang::CallExpr *CE) {
  ParamVector params(CE);
  for (auto param : CE->arguments()) {
    param = param->IgnoreParenCasts();
    if (auto intLit = dyn_cast<IntegerLiteral>(param))
      params.push_back(new IntegerParam(intLit));
    else if (auto strLit = dyn_cast<StringLiteral>(param))
      params.push_back(new StringParam(strLit));
    else if (auto var = dyn_cast<DeclRefExpr>(param)) {
      if (isa<EnumConstantDecl>(var->getDecl()))
        params.push_back(new EnumParam(var));
      else
        params.push_back(new VariableParam(var));
    } else
      params.push_back(new SFParam(param));
  }
  return params;
}

llvm::SmallVector<ParamVector>
DescriptionManager::getParams(llvm::StringRef SFName,
                              const FunctionDecl *Function) {
  llvm::StringMap<llvm::SmallVector<ParamVector>> sFunctions;
  if (!Function->hasBody())
    return llvm::SmallVector<ParamVector>();
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
