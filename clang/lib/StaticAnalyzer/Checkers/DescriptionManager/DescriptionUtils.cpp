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

SmallVector<unsigned> DescriptionUtils::FindParameters(
    BugReporter &BR, AnalysisDeclContext *currentADC,
    const FunctionDecl *currentFunction, ParamVector params,
    CheckerNameRef checkerName) {
  SmallVector<unsigned> args;
  for (auto elem : params) {
    if (auto param = dyn_cast<VariableParam>(&*elem)) {
      int argIndex = LookForArgument(currentFunction, param->GetName());
      if (argIndex == -1) {
        auto pdl = PathDiagnosticLocation(
            param->GetItemLocation(),
            currentADC->getASTContext().getSourceManager());
        BR.EmitBasicReport(
            currentADC->getDecl(), checkerName,
            "Incorrect Configuration Parameter", "Incorrect Configuration",
            llvm::formatv("{0} is not an argument of {1}", param->GetName(),
                          currentFunction->getName())
                .str(),
            pdl);
      } else
        args.push_back(argIndex);
    } else {
      auto pdl = PathDiagnosticLocation(
          elem->GetItemLocation(),
          currentADC->getASTContext().getSourceManager());
      BR.EmitBasicReport(currentADC->getDecl(), checkerName,
          "Incorrect Configuration Parameter", "Incorrect Configuration",
          "Expected a variable here", pdl);
    }
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
