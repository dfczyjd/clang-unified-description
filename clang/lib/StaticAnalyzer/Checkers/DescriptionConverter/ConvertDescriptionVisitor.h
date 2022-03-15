#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKER_DESCRIPTIONCONVERTER_CONVERTDESCRIPTIONVISITOR_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKER_DESCRIPTIONCONVERTER_CONVERTDESCRIPTIONVISITOR_H

#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/AST/StmtVisitor.h"
#include "DescriptionConverter.h"

using namespace clang;
using namespace clang::ento;

class ConvertDescriptionVisitor
    : public ConstStmtVisitor<ConvertDescriptionVisitor> {
  BugReporter &BR;
  AnalysisDeclContext *ADC;
  const CheckerBase *C;
  std::vector<DescriptionConverter *> Converters;
  void VisitChildren(const Stmt *S);

public:
  ConvertDescriptionVisitor(BugReporter &BR, AnalysisDeclContext *ADC,
                            const CheckerBase *C,
                            std::vector<DescriptionConverter *> Converters)
      : BR(BR), ADC(ADC), C(C), Converters(Converters) {}

  void VisitCallExpr(const CallExpr *ce);
  void VisitStmt(const Stmt *S);
  void GenerateError(const char *converter, const char *message,
                     SourceLocation *location = nullptr) const;
};

#endif // LLVM_CLANG_LIB_STATICANALYZER_CHECKER_DESCRIPTIONCONVERTER_CONVERTDESCRIPTIONVISITOR_H
