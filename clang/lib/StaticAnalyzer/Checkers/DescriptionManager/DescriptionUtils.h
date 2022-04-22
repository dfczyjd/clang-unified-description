#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_DESCRIPTIONUTILS_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_DESCRIPTIONUTILS_H

#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "SFParam.h"
#include "DescriptionManager.h"
#include <set>

static class DescriptionUtils {
public:
  static int LookForArgument(const clang::FunctionDecl *function,
                             llvm::StringRef argName);

  static llvm::SmallVector<unsigned>
  FindParameters(clang::ento::BugReporter &BR,
                 clang::AnalysisDeclContext *currentADC,
                 const clang::FunctionDecl *currentFunction, ParamVector params,
                 CheckerNameRef checkerName);

  static bool IsParsedBy(const CheckerBase *C,
                         const clang::FunctionDecl *function);
  static void MarkAsParsedBy(const CheckerBase *C,
                             const clang::FunctionDecl *function);
  static void EmitBugReport(clang::ento::BugReporter &BR,
                            clang::AnalysisDeclContext *ADC,
                            clang::ento::CheckerNameRef checkerName,
                            SFParam *paramWithIssue, llvm::StringRef bugStr);
  static void EmitBugReport(clang::ento::BugReporter &BR,
                            clang::AnalysisDeclContext *ADC,
                            clang::ento::CheckerNameRef checkerName,
                            clang::SourceLocation callWithIssue,
                            llvm::StringRef bugStr);

private:
  static std::set<std::pair<const CheckerBase *, const FunctionDecl *>>
      parsedFunctions;
};

#endif
