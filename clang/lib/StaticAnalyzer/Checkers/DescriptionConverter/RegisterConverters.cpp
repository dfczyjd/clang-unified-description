#include "RegisterConverters.h"
#include "TaintCheckerDescriptionConverter.h"

void RegisterConverters(std::vector<DescriptionConverter*>& Converters) {
  // Add new converters here as the example below
  Converters.push_back(new TaintCheckerDescriptionConverter());
}