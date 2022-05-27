#include "DescriptionUtils.h"
#include "clang/Analysis/PathDiagnostic.h"
#include "llvm/Support/FormatVariadic.h"

using namespace clang;
using namespace ento;

std::set<std::pair<const CheckerBase *, const FunctionDecl *>>
    DescriptionUtils::parsedFunctions;

int DescriptionUtils::lookForArgument(const clang::FunctionDecl *function,
                                        llvm::StringRef argName) {
  int i = 0;
  for (auto it = function->param_begin(), end = function->param_end();
       it != end; ++it, ++i) {
    if ((*it)->getName() == argName)
      return i;
  }
  return -1;
}

void DescriptionUtils::emitBugReport(BugReporter &BR, AnalysisDeclContext *ADC,
                                     CheckerNameRef checkerName,
                                     SourceLocation callWithIssue,
                                     llvm::StringRef bugStr) {
  auto pdl = PathDiagnosticLocation(callWithIssue,
                                    ADC->getASTContext().getSourceManager());
  BR.EmitBasicReport(ADC->getDecl(), checkerName, "Incorrect SF Call",
                     "Incorrect Configuration", bugStr, pdl);
}

void DescriptionUtils::emitBugReport(BugReporter &BR, AnalysisDeclContext *ADC,
                                     CheckerNameRef checkerName,
                                     SFParam *paramWithIssue,
                                     llvm::StringRef bugStr) {
  auto pdl = PathDiagnosticLocation(paramWithIssue->getItemLocation(),
                                    ADC->getASTContext().getSourceManager());
  BR.EmitBasicReport(ADC->getDecl(), checkerName,
                     "Incorrect SF Parameter",
                     "Incorrect Configuration",
                     bugStr,
                     pdl);
}

SmallVector<unsigned> DescriptionUtils::findParameters(
    BugReporter &BR, AnalysisDeclContext *currentADC,
    const FunctionDecl *currentFunction, ParamVector params,
    CheckerNameRef checkerName) {
  SmallVector<unsigned> args;
  for (auto elem : params) {
    if (auto param = dyn_cast<VariableParam>(&*elem)) {
      int argIndex = lookForArgument(currentFunction, param->getName());
      if (argIndex == -1)
        emitBugReport(BR, currentADC, checkerName, param,
                      llvm::formatv("{0} is not an argument of {1}",
                                    param->getName(),
                                    currentFunction->getName()).str());
      else
        args.push_back(argIndex);
    } else
      emitBugReport(BR, currentADC, checkerName, elem,
                    "Expected a variable here");
  }
  return args;
}

bool DescriptionUtils::isParsedBy(const CheckerBase *C,
                                  const FunctionDecl *function) {
  return parsedFunctions.count(std::make_pair(C, function));
}

void DescriptionUtils::markAsParsedBy(const CheckerBase *C,
                                      const FunctionDecl *function) {
  parsedFunctions.insert(std::make_pair(C, function));
}
