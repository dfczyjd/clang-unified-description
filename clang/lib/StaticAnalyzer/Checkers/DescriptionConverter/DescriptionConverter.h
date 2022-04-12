#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKER_DESCRIPTIONCONVERTER_DESCRIPTIONCONVERTER_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKER_DESCRIPTIONCONVERTER_DESCRIPTIONCONVERTER_H

#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
class DescriptionConverter;
#include "ConvertDescriptionVisitor.h"

class DescriptionConverter {
public:
  virtual void ProcessFunction(const clang::CallExpr *ce,
                               clang::AnalysisDeclContext *ADC) = 0;

  virtual std::string OutputConfiguration() const = 0;

  void SetWalker(const ConvertDescriptionVisitor* newWalker) { walker = newWalker; }

  llvm::StringRef ConfigDirectory;

protected:
  const ConvertDescriptionVisitor *walker;

  int LookForArgument(const clang::FunctionDecl *function,
                                            llvm::StringRef &argName);

  const clang::DeclRefExpr* UnwrapVariable(const clang::Expr *e) const;

  void GenerateError(const char *converter, const char *message,
                     llvm::Optional<clang::SourceLocation> location) const;
};

#endif // LLVM_CLANG_LIB_STATICANALYZER_CHECKER_DESCRIPTIONCONVERTER_DESCRIPTIONCONVERTER_H
