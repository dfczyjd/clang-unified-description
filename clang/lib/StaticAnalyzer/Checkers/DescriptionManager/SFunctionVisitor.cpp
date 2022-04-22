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
      ParamVector params;
      params.SetSFCallExpr(ce);
      for (auto param : ce->arguments()) {
        param = param->IgnoreParenCasts();
        if (auto intLit = dyn_cast<IntegerLiteral>(param))
          params.push_back(new IntegerParam(intLit));
        else if (auto strLit = dyn_cast<StringLiteral>(param))
          params.push_back(new StringParam(strLit));
        else if (auto var = dyn_cast<DeclRefExpr>(param)) {
          if (isa<EnumConstantDecl>(var->getDecl()))
            params.push_back(new EnumParam(var));
          else
            params.push_back(new VariableParam(var));
        } else
          params.push_back(new SFParam(param));
      }
      res.first->getValue().push_back(params);
    }
  }

  VisitChildren(ce);
}
