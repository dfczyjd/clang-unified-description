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

public:
  SFParam() : Kind(PK_None) {}
  SFParam(ParamKind kind) : Kind(kind) {}
  virtual clang::SourceLocation GetItemLocation() const = 0;

  virtual ~SFParam() = default;

  ParamKind getKind() const { return Kind; }
};

class VariableParam : public SFParam {
public:
  VariableParam(const clang::DeclRefExpr *DE)
      : SFParam(PK_Variable), Declaration(DE) {}

  llvm::StringRef GetName() const { return Declaration->getDecl()->getName(); }
  clang::SourceLocation GetItemLocation() const override {
    return Declaration->getExprLoc();
  }

  static bool classof(const SFParam *P) { return P->getKind() == PK_Variable; }

private:
  const clang::DeclRefExpr *Declaration;
};

class IntegerParam : public SFParam {
public:
  IntegerParam(const clang::IntegerLiteral *L)
      : SFParam(PK_Integer), Literal(L) {}

  llvm::APInt GetValue() const { return Literal->getValue(); }
  clang::SourceLocation GetItemLocation() const override {
    return Literal->getExprLoc();
  }

  static bool classof(const SFParam *P) { return P->getKind() == PK_Integer; }

private:
  const clang::IntegerLiteral *Literal;
};

class StringParam : public SFParam {
public:
  StringParam(const clang::StringLiteral *L) : SFParam(PK_String), Literal(L) {}
  
  llvm::StringRef GetValue() const { return Literal->getString(); }
  clang::SourceLocation GetItemLocation() const override {
    return Literal->getExprLoc();
  }

  static bool classof(const SFParam *P) { return P->getKind() == PK_String; }

private:
  const clang::StringLiteral *Literal;
};

class EnumParam : public SFParam {
public:
  EnumParam(const clang::DeclRefExpr *DE)
      : SFParam(PK_Enum), Declaration(DE),
        Value(llvm::dyn_cast<clang::EnumConstantDecl>(DE->getDecl())) {}

  llvm::APInt GetValue() const {
    return Value->getInitVal();
  }
  clang::SourceLocation GetItemLocation() const override {
    return Declaration->getExprLoc();
  }
  bool isCorrect() const { return Value != nullptr; }

  static bool classof(const SFParam *P) { return P->getKind() == PK_Enum; }

private:
  const clang::DeclRefExpr *Declaration;
  const clang::EnumConstantDecl *Value;
};

class ParamVector : public llvm::SmallVector<SFParam *> {
public:
  void SetSFCallExpr(const clang::CallExpr *CE) { SFCall = CE; }

  clang::SourceLocation GetCallLocation() const {
    return SFCall->getExprLoc();
  }

private:
  const clang::CallExpr *SFCall;
};

#endif
