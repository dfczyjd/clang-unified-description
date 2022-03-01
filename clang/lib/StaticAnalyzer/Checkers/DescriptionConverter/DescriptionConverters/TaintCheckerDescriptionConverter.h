#pragma once
#include "DescriptionConverter.h"
#include "llvm/Support/YAMLTraits.h"

using namespace clang;
using namespace ento;

#pragma region YAML
// Type definitions from checker cpp file
using ArgVector = SmallVector<unsigned, 2>;
using SignedArgVector = SmallVector<int, 2>;

enum class VariadicType { None, Src, Dst };

static const unsigned InvalidArgIndex{std::numeric_limits<unsigned>::max()};

struct TaintConfiguration {
  using NameScopeArgs = std::tuple<std::string, std::string, ArgVector>;

  struct Propagation {
    std::string Name;
    std::string Scope;
    ArgVector SrcArgs;
    SignedArgVector DstArgs;
    VariadicType VarType;
    unsigned VarIndex;
  };

  std::vector<Propagation> Propagations;
  std::vector<NameScopeArgs> Filters;
  std::vector<NameScopeArgs> Sinks;

  TaintConfiguration() = default;
  TaintConfiguration(const TaintConfiguration &) = default;
  TaintConfiguration(TaintConfiguration &&) = default;
  TaintConfiguration &operator=(const TaintConfiguration &) = default;
  TaintConfiguration &operator=(TaintConfiguration &&) = default;
};

LLVM_YAML_IS_SEQUENCE_VECTOR(TaintConfiguration::Propagation)
LLVM_YAML_IS_SEQUENCE_VECTOR(TaintConfiguration::NameScopeArgs)

namespace llvm {
namespace yaml {
template <> struct MappingTraits<TaintConfiguration> {
  static void mapping(IO &IO, TaintConfiguration &Config) {
    IO.mapOptional("Propagations", Config.Propagations);
    IO.mapOptional("Filters", Config.Filters);
    IO.mapOptional("Sinks", Config.Sinks);
  }
};

template <> struct MappingTraits<TaintConfiguration::Propagation> {
  static void mapping(IO &IO, TaintConfiguration::Propagation &Propagation) {
    IO.mapRequired("Name", Propagation.Name);
    IO.mapOptional("Scope", Propagation.Scope);
    IO.mapOptional("SrcArgs", Propagation.SrcArgs);
    IO.mapOptional("DstArgs", Propagation.DstArgs);
    IO.mapOptional("VariadicType", Propagation.VarType, VariadicType::None);
    IO.mapOptional("VariadicIndex", Propagation.VarIndex, InvalidArgIndex);
  }
};

template <> struct ScalarEnumerationTraits<VariadicType> {
  static void enumeration(IO &IO, VariadicType &Value) {
    IO.enumCase(Value, "None", VariadicType::None);
    IO.enumCase(Value, "Src", VariadicType::Src);
    IO.enumCase(Value, "Dst", VariadicType::Dst);
  }
};

template <> struct MappingTraits<TaintConfiguration::NameScopeArgs> {
  static void mapping(IO &IO, TaintConfiguration::NameScopeArgs &NSA) {
    IO.mapRequired("Name", std::get<0>(NSA));
    IO.mapOptional("Scope", std::get<1>(NSA));
    IO.mapRequired("Args", std::get<2>(NSA));
  }
};
} // namespace yaml
} // namespace llvm
#pragma endregion

class TaintCheckerDescriptionConverter : public DescriptionConverter {
public:
  void ProcessFunction(const CallExpr *ce, AnalysisDeclContext *ADC) override;

  std::string OutputConfiguration() const override;

  TaintConfiguration Config;

private:
  ArgVector ConvertArgList(const FunctionDecl *currentFunction,
                           const CallExpr *ce);

  const llvm::StringRef CONFIG_FILE = "config.yaml";
};