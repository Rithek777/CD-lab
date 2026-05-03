#pragma once

#include <string>

namespace fap {

enum class AllocationClassification {
  ProvablyUnnecessary,
  PossiblyUnnecessary,
  Necessary
};

struct AllocationInfo {
  AllocationClassification classification =
      AllocationClassification::PossiblyUnnecessary;
  std::string reason;
  double confidence = 0.0;
  std::string suggestedFix;
};

const char *toString(AllocationClassification classification);

} // namespace fap

