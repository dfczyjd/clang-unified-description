#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "llvm/Support/FormatVariadic.h"
#include "DescriptionManager/DescriptionManager.h"
#include "DescriptionManager/DescriptionUtils.h"
#include "MallocChecker.h"

using namespace clang;
using namespace clang::ento;

namespace {
class SpecialFunctionParser : public Checker<check::PostCall> {
  mutable std::unique_ptr<BugType> BT;

public:
  void checkPostCall(const CallEvent &Call, CheckerContext &Ctx) const;

private:
#define SF_PROCESSOR(NAME)                                                     \
  void NAME##_process(const CallEvent &Call, CheckerContext &Ctx) const;

  SF_PROCESSOR(sf_allocate_with_size)

  using SFProcessFn = std::function<void(
      const SpecialFunctionParser *, const CallEvent &Call, CheckerContext &C)>;
  const CallDescriptionMap<SFProcessFn> SFProcessors{
      {{"sf_allocate_with_size", 2}, &SpecialFunctionParser::sf_allocate_with_size_process},
  };
};
} // namespace

void SpecialFunctionParser::checkPostCall(const CallEvent &Call,
                                          CheckerContext &Ctx) const {
  if (auto callback = SFProcessors.lookup(Call))
    (*callback)(this, Call, Ctx);
}

void SpecialFunctionParser::sf_allocate_with_size_process(
    const CallEvent &Call, CheckerContext &Ctx) const {
  const CallExpr *CE;
  if (!(CE = dyn_cast<CallExpr>(Call.getOriginExpr())))
    return;
  ParamVector params = DescriptionManager::ProcessCallExpr(CE);
  if (params.size() != 2) {
    DescriptionUtils::EmitBugReport(
        Ctx.getBugReporter(), Ctx.getCurrentAnalysisDeclContext(),
        getCheckerName(), params.GetCallLocation(),
        "This function should have exactly 2 arguments");
    return;
  }
  if (!params[0]->GetExpr()->getType().getTypePtr()->isIntegerType()) {
    DescriptionUtils::EmitBugReport(
        Ctx.getBugReporter(), Ctx.getCurrentAnalysisDeclContext(),
        getCheckerName(), params[0],
        "Expected an expression of integer type here");
    return;
  }

  if (auto allocFamilyArg = dyn_cast<EnumParam>(params[1])) {
    auto allocFamily =
        AllocationFamily(allocFamilyArg->GetValue().getZExtValue());

    unsigned Count = Ctx.blockCount();
    auto State = Ctx.getState();
    auto LCtx = Ctx.getLocationContext();
    SVal Size = State->getSVal(Call.getArgExpr(0), LCtx);

    if (Size.isUndef())
      return;

    auto &svalBuilder = Ctx.getSValBuilder();
    auto arrayWithSize = svalBuilder.getConjuredHeapSymbolVal(CE, LCtx, Count)
                              .castAs<DefinedSVal>();
    State = State->BindExpr(CE, Ctx.getLocationContext(), arrayWithSize);
    State =
        setDynamicExtent(State, arrayWithSize.getAsRegion(),
                          Size.castAs<DefinedOrUnknownSVal>(), svalBuilder);

    auto zero = nonloc::ConcreteInt(llvm::APSInt::get(0));
    auto sizeCmp = Ctx.getSValBuilder().evalEQ(State, Size, zero);
    
    // Size is defined, so "Size == 0" should also be
    assert(!sizeCmp.isUndef());

    ProgramStateRef zeroState, nonZeroState;
    std::tie(zeroState, nonZeroState) = State->assume(sizeCmp.castAs<DefinedOrUnknownSVal>());
    
    auto allocCallExpr = Call.getCalleeStackFrame(Count)
                             ->getParent()
                             ->getStackFrame()
                             ->getCallSite();
    if (zeroState && !nonZeroState)
      State = setRegionZeroAllocated(State, arrayWithSize.getAsSymbol(),
                                     allocFamily, allocCallExpr);
    else
      State = setRegionAllocated(State, arrayWithSize.getAsSymbol(),
                                 allocFamily, allocCallExpr);
    Ctx.addTransition(State);
  } else {
    DescriptionUtils::EmitBugReport(
        Ctx.getBugReporter(), Ctx.getCurrentAnalysisDeclContext(),
        getCheckerName(), params[1]->GetItemLocation(),
        "Expected a SF_AllocationFamily enum here");
  }
}

namespace clang {
namespace ento {
bool shouldRegisterSpecialFunctionParser(const CheckerManager &Mgr) {
  return true;
}

void registerSpecialFunctionParser(CheckerManager &Mgr) {
  Mgr.registerChecker<SpecialFunctionParser>();
}
} // namespace ento
} // namespace clang
