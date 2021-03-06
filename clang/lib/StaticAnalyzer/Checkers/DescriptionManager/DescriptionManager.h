#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_DESCRIPTIONMANAGER_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_DESCRIPTIONMANAGER_H

#include "SFunctionVisitor.h"
#include "llvm/ADT/StringSet.h"

class DescriptionManager {

public:
  static void addSFunction(llvm::StringRef SFName) {
    RegisteredFunctions.insert(SFName);
  }

  /// <summary>
  /// Looks for named description function in the body of the given function
  /// </summary>
  /// <param name="SFName">The name of the description function</param>
  /// <param name="Function">The function to look in</param>
  /// <returns>List of parameters of found description function</returns>
  static llvm::SmallVector<ParamVector> getParams(llvm::StringRef SFName,
                                                  const FunctionDecl *Function);

  static void setWalker(SFunctionVisitor *Walker);

  static llvm::StringSet<> &getRegisteredFunctions() {
    return RegisteredFunctions;
  }

  static ParamVector processCallExpr(const clang::CallExpr *CE);

private:
  static SFunctionVisitor *Walker;
  static llvm::StringSet<> RegisteredFunctions;
  static std::map<const FunctionDecl *,
                  llvm::StringMap<llvm::SmallVector<ParamVector>>>
      SFParams;
};

#endif
