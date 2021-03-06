#include "DescriptionConverter.h"

int DescriptionConverter::lookForArgument(
    const clang::FunctionDecl *function, llvm::StringRef &argName) {
  int i = 0;
  for (auto it = function->param_begin(), end = function->param_end();
       it != end; ++it, ++i) {
    if ((*it)->getName() == argName)
      return i;
  }
  return -1;
}

const clang::DeclRefExpr *
DescriptionConverter::unwrapVariable(const clang::Expr *e) const {
  auto _1 = llvm::dyn_cast<clang::ImplicitCastExpr>(e);
  if (_1 == nullptr)
    return nullptr;
  return llvm::dyn_cast<clang::DeclRefExpr>(*_1->child_begin());
}

void DescriptionConverter::generateError(const char *converter,
                                         const char *message,
                                         llvm::Optional<clang::SourceLocation> location) const {
  walker->GenerateError(converter, message, location);
}
