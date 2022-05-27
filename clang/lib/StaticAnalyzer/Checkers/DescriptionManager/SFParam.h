#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_SFPARAM_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_DESCRIPTIONMANAGER_SFPARAM_H

#include "clang/StaticAnalyzer/Core/Checker.h"
#include "llvm/Support/Casting.h"

// Base class for sf_ functions' parameters
class SFParam {
public:
  enum ParamKind {
    PK_None,
    PK_Variable,
    PK_Integer,
    PK_String,
    PK_Enum
  };

private:
  ParamKind Kind;

  const clang::Expr *Expression;

public:
  SFParam(const clang::Expr* E = nullptr) : Kind(PK_None), Expression(E) {}
  SFParam(ParamKind kind) : Kind(kind), Expression(nullptr) {}
  virtual clang::SourceLocation getItemLocation() const {
    return Expression->getExprLoc();
  }

  virtual const clang::Expr *getExpr() const { return Expression; }

  static bool classof(const SFParam *P) { return P->getKind() == PK_None; }
  virtual ~SFParam() = default;
  ParamKind getKind() const { return Kind; }
};

class VariableParam : public SFParam {
public:
  VariableParam(const clang::DeclRefExpr *DE)
      : SFParam(PK_Variable), Expression(DE) {}

  llvm::StringRef getName() const { return Expression->getDecl()->getName(); }
  const clang::DeclRefExpr *getExpr() const override { return Expression; }
  clang::SourceLocation getItemLocation() const override {
    return Expression->getExprLoc();
  }

  static bool classof(const SFParam *P) { return P->getKind() == PK_Variable; }

private:
  const clang::DeclRefExpr *Expression;
};

class IntegerParam : public SFParam {
public:
  IntegerParam(const clang::IntegerLiteral *L)
      : SFParam(PK_Integer), Expression(L) {}

  llvm::APInt getValue() const { return Expression->getValue(); }
  const clang::IntegerLiteral *getExpr() const override { return Expression; }

  static bool classof(const SFParam *P) { return P->getKind() == PK_Integer; }

private:
  const clang::IntegerLiteral *Expression;
};

class StringParam : public SFParam {
public:
  StringParam(const clang::StringLiteral *L)
      : SFParam(PK_String), Expression(L) {}
  
  llvm::StringRef getValue() const { return Expression->getString(); }

  static bool classof(const SFParam *P) { return P->getKind() == PK_String; }

private:
  const clang::StringLiteral *Expression;
};

class EnumParam : public SFParam {
public:
  EnumParam(const clang::DeclRefExpr *DE)
      : SFParam(PK_Enum), Expression(DE),
        Value(llvm::dyn_cast<clang::EnumConstantDecl>(DE->getDecl())) {}

  llvm::APInt getValue() const {
    return Value->getInitVal();
  }
  bool isCorrect() const { return Value != nullptr; }

  static bool classof(const SFParam *P) { return P->getKind() == PK_Enum; }

private:
  const clang::DeclRefExpr *Expression;
  const clang::EnumConstantDecl *Value;
};

class ParamVector : public llvm::SmallVector<SFParam *> {
public:
  ParamVector(const clang::CallExpr *CE)
      : llvm::SmallVector<SFParam *>(), SFCall(CE) {}

  const clang::CallExpr *getSFCallExpr() { return SFCall; }

  clang::SourceLocation getCallLocation() const {
    return SFCall->getExprLoc();
  }

private:
  const clang::CallExpr *SFCall;
};

#endif
