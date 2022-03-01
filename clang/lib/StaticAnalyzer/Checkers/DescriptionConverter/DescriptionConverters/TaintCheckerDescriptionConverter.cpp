#include "DescriptionConverter.h"
#include "llvm/Support/YAMLTraits.h"
#include "TaintCheckerDescriptionConverter.h"
#include "llvm/Support/FormatVariadic.h"

ArgVector TaintCheckerDescriptionConverter::ConvertArgList(const FunctionDecl *currentFunction,
    const CallExpr *ce) {
  ArgVector args;
  for (auto it = ce->arg_begin(), end = ce->arg_end(); it != end; ++it) {
    if (auto arg = UnwrapVariable(*it)) {
      auto argName = arg->getDecl()->getName();
      int argIndex = LookForArgument(currentFunction, argName);
      if (argIndex == -1) {
        auto error = llvm::formatv("{0} is not an argument of {1}", argName,
                                   currentFunction->getName())
                         .str();
        auto argLocation = (*it)->getExprLoc();
        GenerateError("TaintCheckerConverter", error.c_str(), &argLocation);
      } else {
        args.push_back(argIndex);
      }
    } else {
      auto argLocation = (*it)->getExprLoc();
      GenerateError("TaintCheckerConverter", "Expected a variable here",
                    &argLocation);
    }
  }
  return args;
}

void TaintCheckerDescriptionConverter::ProcessFunction(
    const clang::CallExpr *ce, AnalysisDeclContext *ADC) {
  auto function = ce->getCalleeDecl()->getAsFunction();
  if (function->getName() == "sf_propagation_src") {
    auto currentFunc = ADC->getDecl()->getAsFunction();
    ArgVector srcArgs = ConvertArgList(currentFunc, ce);
    for (auto prop : Config.Propagations) {
      if (prop.Name == currentFunc->getName()) {
        prop.SrcArgs.insert(prop.SrcArgs.end(), srcArgs.begin(), srcArgs.end());
        return;
      }
    }
    TaintConfiguration::Propagation prop;
    prop.Name = currentFunc->getName().str();
    prop.SrcArgs = srcArgs;
    prop.VarType = VariadicType::None;
    prop.VarIndex = InvalidArgIndex;
    Config.Propagations.push_back(prop);
  } else if (function->getName() == "sf_propagation_dst") {
    auto currentFunc = ADC->getDecl()->getAsFunction();
    SignedArgVector dstArgs;
    ArgVector args = ConvertArgList(currentFunc, ce);
    dstArgs.append(args.begin(), args.end());
    for (auto& prop : Config.Propagations) {
      if (prop.Name == currentFunc->getName()) {
        prop.DstArgs.insert(prop.DstArgs.end(), dstArgs.begin(), dstArgs.end());
        return;
      }
    }
    TaintConfiguration::Propagation prop;
    prop.Name = currentFunc->getName().str();
    prop.DstArgs = dstArgs;
    prop.VarType = VariadicType::None;
    prop.VarIndex = InvalidArgIndex;
    Config.Propagations.push_back(prop);
  } else if (function->getName() == "sf_propagation_return") {
    auto currentFunc = ADC->getDecl()->getAsFunction();
    for (auto &prop : Config.Propagations) {
      if (prop.Name == currentFunc->getName()) {
        prop.DstArgs.push_back(-1);
        return;
      }
    }
    TaintConfiguration::Propagation prop;
    prop.Name = currentFunc->getName().str();
    prop.DstArgs = {-1};
    prop.VarType = VariadicType::None;
    prop.VarIndex = InvalidArgIndex;
    Config.Propagations.push_back(prop);
  } else if (function->getName() == "sf_sink") {
    auto currentFunc = ADC->getDecl()->getAsFunction();
    ArgVector sinkArgs = ConvertArgList(currentFunc, ce);
    TaintConfiguration::NameScopeArgs sink(
        {currentFunc->getName().str(), "", sinkArgs});
    Config.Sinks.push_back(sink);
  } else if (function->getName() == "sf_filter") {
    auto currentFunc = ADC->getDecl()->getAsFunction();
    ArgVector filterArgs = ConvertArgList(currentFunc, ce);
    TaintConfiguration::NameScopeArgs filter(
        {currentFunc->getName().str(), "", filterArgs});
    Config.Filters.push_back(filter);
  }
}

std::string TaintCheckerDescriptionConverter::OutputConfiguration() const {
  std::error_code error;
  std::string configFilename;
  if (ConfigDirectory.empty())
    configFilename = CONFIG_FILE.str();
  else
    configFilename = (ConfigDirectory + "/" + CONFIG_FILE).str();
  llvm::raw_fd_ostream fout(configFilename, error);
  llvm::yaml::Output out(fout);
  TaintConfiguration outConfig = Config;
  out << outConfig;
  auto cmdParam =
      ("-analyzer-config alpha.security.taint.TaintPropagation:Config=" +
      llvm::StringRef(configFilename)).str();
  return cmdParam;
}