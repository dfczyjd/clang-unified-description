#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_MALLOCCHECKER_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_MALLOCCHECKER_H

using namespace clang;
using namespace ento;

//===----------------------------------------------------------------------===//
// The types of allocation we're modeling. This is used to check whether a
// dynamically allocated object is deallocated with the correct function, like
// not using operator delete on an object created by malloc(), or alloca regions
// aren't ever deallocated manually.
//===----------------------------------------------------------------------===//

// Used to check correspondence between allocators and deallocators.
enum AllocationFamily {
  AF_None,
  AF_Malloc,
  AF_CXXNew,
  AF_CXXNewArray,
  AF_IfNameIndex,
  AF_Alloca,
  AF_InnerBuffer
};

//===----------------------------------------------------------------------===//
// The state of a symbol, in terms of memory management.
//===----------------------------------------------------------------------===//

namespace {
class RefState {
  enum Kind {
    // Reference to allocated memory.
    Allocated,
    // Reference to zero-allocated memory.
    AllocatedOfSizeZero,
    // Reference to released/freed memory.
    Released,
    // The responsibility for freeing resources has transferred from
    // this reference. A relinquished symbol should not be freed.
    Relinquished,
    // We are no longer guaranteed to have observed all manipulations
    // of this pointer/memory. For example, it could have been
    // passed as a parameter to an opaque function.
    Escaped
  };

  const Stmt *S;

  Kind K;
  AllocationFamily Family;

  RefState(Kind k, const Stmt *s, AllocationFamily family)
      : S(s), K(k), Family(family) {
    assert(family != AF_None);
  }

public:
  bool isAllocated() const { return K == Allocated; }
  bool isAllocatedOfSizeZero() const { return K == AllocatedOfSizeZero; }
  bool isReleased() const { return K == Released; }
  bool isRelinquished() const { return K == Relinquished; }
  bool isEscaped() const { return K == Escaped; }
  AllocationFamily getAllocationFamily() const { return Family; }
  const Stmt *getStmt() const { return S; }

  bool operator==(const RefState &X) const {
    return K == X.K && S == X.S && Family == X.Family;
  }

  static RefState getAllocated(AllocationFamily family, const Stmt *s) {
    return RefState(Allocated, s, family);
  }
  static RefState getAllocatedOfSizeZero(const RefState *RS) {
    return RefState(AllocatedOfSizeZero, RS->getStmt(),
                    RS->getAllocationFamily());
  }
  static RefState getReleased(AllocationFamily family, const Stmt *s) {
    return RefState(Released, s, family);
  }
  static RefState getRelinquished(AllocationFamily family, const Stmt *s) {
    return RefState(Relinquished, s, family);
  }
  static RefState getEscaped(const RefState *RS) {
    return RefState(Escaped, RS->getStmt(), RS->getAllocationFamily());
  }

  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(K);
    ID.AddPointer(S);
    ID.AddInteger(Family);
  }

  LLVM_DUMP_METHOD void dump(raw_ostream &OS) const {
    switch (K) {
#define CASE(ID)                                                               \
  case ID:                                                                     \
    OS << #ID;                                                                 \
    break;
      CASE(Allocated)
      CASE(AllocatedOfSizeZero)
      CASE(Released)
      CASE(Relinquished)
      CASE(Escaped)
    }
  }

  LLVM_DUMP_METHOD void dump() const { dump(llvm::errs()); }
};

} // end of anonymous namespace

ProgramStateRef setRegionZeroAllocated(ProgramStateRef state, SymbolRef region,
                                       AllocationFamily family, const Stmt *S);

ProgramStateRef setRegionAllocated(ProgramStateRef state,
                                          SymbolRef region, AllocationFamily family, const Stmt* S);

#endif
