#include "RegisterConverters.h"
#include "DescriptionConverters/TaintCheckerDescriptionConverter.h"

void registerConverters(std::vector<DescriptionConverter*>& Converters) {
  // Add new converters here as the example below
  Converters.push_back(new TaintCheckerDescriptionConverter());
}