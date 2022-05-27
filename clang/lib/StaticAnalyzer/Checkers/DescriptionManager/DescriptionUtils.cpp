#include "DescriptionUtils.h"
#include "clang/Analysis/PathDiagnostic.h"
#include "llvm/Support/FormatVariadic.h"

using namespace clang;
using namespace ento;

std::set<std::pair<const CheckerBase *, const FunctionDecl *>>
    DescriptionUtils::parsedFunctions;

int DescriptionUtils::LookForArgument(const clang::FunctionDecl *function,
                                        llvm::StringRef argName) {
  int i = 0;
  for (auto it = function->param_begin(), end = function->param_end();
       it != end; ++it, ++i) {
    if ((*it)->getName() == argName)
      return i;
  }
  return -1;
}

void DescriptionUtils::EmitBugReport(BugReporter &BR, AnalysisDeclContext *ADC,
                                     CheckerNameRef checkerName,
                                     SourceLocation callWithIssue,
                                     llvm::StringRef bugStr) {
  auto pdl = PathDiagnosticLocation(callWithIssue,
                                    ADC->getASTContext().getSourceManager());
  BR.EmitBasicReport(ADC->getDecl(), checkerName, "Incorrect SF Call",
                     "Incorrect Configuration", bugStr, pdl);
}

void DescriptionUtils::EmitBugReport(BugReporter &BR, AnalysisDeclContext *ADC,
                                     CheckerNameRef checkerName,
                                     SFParam *paramWithIssue,
                                     llvm::StringRef bugStr) {
  auto pdl = PathDiagnosticLocation(paramWithIssue->GetItemLocation(),
                                    ADC->getASTContext().getSourceManager());
  BR.EmitBasicReport(ADC->getDecl(), checkerName,
                     "Incorrect SF Parameter",
                     "Incorrect Configuration",
                     bugStr,
                     pdl);
}

SmallVector<unsigned> DescriptionUtils::FindParameters(
    BugReporter &BR, AnalysisDeclContext *currentADC,
    const FunctionDecl *currentFunction, ParamVector params,
    CheckerNameRef checkerName) {
  SmallVector<unsigned> args;
  for (auto elem : params) {
    if (auto param = dyn_cast<VariableParam>(&*elem)) {
      int argIndex = LookForArgument(currentFunction, param->GetName());
      if (argIndex == -1)
        EmitBugReport(BR, currentADC, checkerName, param,
                      llvm::formatv("{0} is not an argument of {1}",
                                    param->GetName(),
                                    currentFunction->getName()).str());
      else
        args.push_back(argIndex);
    } else
      EmitBugReport(BR, currentADC, checkerName, elem,
                    "Expected a variable here");
  }
  return args;
}

bool DescriptionUtils::IsParsedBy(const CheckerBase *C,
                                  const FunctionDecl *function) {
  return parsedFunctions.count(std::make_pair(C, function));
}

void DescriptionUtils::MarkAsParsedBy(const CheckerBase *C,
                                      const FunctionDecl *function) {
  parsedFunctions.insert(std::make_pair(C, function));
}
