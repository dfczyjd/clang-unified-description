#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_SFUNCTIONVISITOR_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_SFUNCTIONVISITOR_H

#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/AST/StmtVisitor.h"
#include "SFParam.h"

using namespace clang;
using namespace clang::ento;

class SFunctionVisitor
    : public ConstStmtVisitor<SFunctionVisitor> {
  BugReporter &BR;
  AnalysisDeclContext *ADC;
  const CheckerBase *C;
  llvm::StringMap<llvm::SmallVector<ParamVector>> FoundSFs;

  void VisitChildren(const Stmt *S);

public:
  SFunctionVisitor(BugReporter &BR, AnalysisDeclContext *ADC,
                            const CheckerBase *C)
      : BR(BR), ADC(ADC), C(C) {}

  ~SFunctionVisitor();

  void VisitCallExpr(const CallExpr *ce);
  void VisitStmt(const Stmt *S);
  const llvm::StringMap<llvm::SmallVector<ParamVector>> &GetFoundSFs() {
    return FoundSFs;
  }
};

#endif
