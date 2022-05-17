#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/FormatVariadic.h"
#include "SFunctionVisitor.h"
#include "DescriptionManager.h"

using namespace clang;
using namespace clang::ento;

void SFunctionVisitor::VisitChildren(const Stmt *S) {
  for (auto I = S->child_begin(), E = S->child_end(); I != E; ++I)
    if (const Stmt *child = *I)
      Visit(child);
}

void SFunctionVisitor::VisitStmt(const Stmt *S) { VisitChildren(S); }

void SFunctionVisitor::VisitCallExpr(const CallExpr *ce) {
  auto funcName = ce->getCalleeDecl()->getAsFunction()->getName();
  auto funcs = DescriptionManager::GetRegisteredFunctions();
  for (auto it = funcs.begin(), end = funcs.end(); it != end; ++it) {
    auto func = it->getKey();
    if (funcName == func) {
      auto res = FoundSFs.try_emplace(func);
      res.first->getValue().push_back(DescriptionManager::ProcessCallExpr(ce));
    }
  }

  VisitChildren(ce);
}
