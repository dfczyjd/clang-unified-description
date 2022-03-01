#pragma once
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "DescriptionConverter.h"
#include "RegisterConverters.h"

using namespace clang;
using namespace clang::ento;

namespace {
class ConvertDescriptionChecker : public Checker<check::ASTCodeBody, check::EndAnalysis> {
public:
  ConvertDescriptionChecker() {
    RegisterConverters(Converters);
  }

  void checkASTCodeBody(const Decl *D, AnalysisManager &Mgr,
                        BugReporter &BR) const;
  void checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const;

  std::vector<DescriptionConverter*> Converters;
  llvm::StringRef ConfigDirectory;
};
} // namespace