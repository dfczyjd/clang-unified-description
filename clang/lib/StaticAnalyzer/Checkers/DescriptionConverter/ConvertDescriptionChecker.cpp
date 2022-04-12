#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/FormatVariadic.h"
#include "DescriptionConverter.h"
#include "ConvertDescriptionChecker.h"
#include "ConvertDescriptionVisitor.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"

using namespace clang;
using namespace clang::ento;

// Generate error node at current location
void ConvertDescriptionVisitor::GenerateError(const char *converter,
                                         const char *message, llvm::Optional<SourceLocation> location) const {
  PathDiagnosticLocation pdl;
  if (location.hasValue())
    pdl = PathDiagnosticLocation(*location,
                                 ADC->getASTContext().getSourceManager());
  else
    // No location known, use the one of the context
    pdl = PathDiagnosticLocation(ADC->getDecl(),
                                 ADC->getASTContext().getSourceManager());
  BR.EmitBasicReport(ADC->getDecl(), C->getCheckerName(), converter, converter,
                     message, pdl);
}

void ConvertDescriptionChecker::checkASTCodeBody(const Decl *D,
                                                 AnalysisManager &Mgr,
                                                 BugReporter &BR) const {
  ConvertDescriptionVisitor walker(
      BR, Mgr.getAnalysisDeclContextManager().getContext(D), this, Converters);
  for (auto conv : Converters) {
    conv->SetWalker(&walker);
    conv->ConfigDirectory = ConfigDirectory;
  }
  walker.Visit(D->getBody());
}

void ConvertDescriptionChecker::checkEndAnalysis(ExplodedGraph &G,
                                                 BugReporter &BR,
                                                 ExprEngine &Eng) const {
  std::string config;
  for (auto conv : Converters) {
    auto convConfig = conv->OutputConfiguration();
    if (convConfig.empty())
      continue;
    config = Twine(config).concat(convConfig).str();
  }
  if (!config.empty()) {
    llvm::outs() << "Add these arguments to your compilation string:\n";
    llvm::outs() << config << '\n';
  }
}

// ConvertDescriptionVisitor

void ConvertDescriptionVisitor::VisitChildren(const Stmt *S) {
  for (auto I = S->child_begin(), E = S->child_end(); I != E; ++I)
    if (const Stmt *child = *I)
      Visit(child);
}

void ConvertDescriptionVisitor::VisitStmt(const Stmt *S) { VisitChildren(S); }

void ConvertDescriptionVisitor::VisitCallExpr(const CallExpr *ce) {
  for (auto conv : Converters) {
    conv->ProcessFunction(ce, ADC);
  }

  VisitChildren(ce);
}

namespace clang {
namespace ento {
bool shouldRegisterConvertDescriptionChecker(const CheckerManager &Mgr) {
  return true;
}

void registerConvertDescriptionChecker(CheckerManager &Mgr) {
  auto Checker = Mgr.registerChecker<ConvertDescriptionChecker>();
  Checker->ConfigDirectory = Mgr.getAnalyzerOptions().getCheckerStringOption(Checker, "OutputDir");
}
} // namespace ento
} // namespace clang