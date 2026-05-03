#pragma once

#include "FAP/AllocationKind.h"

#include <cstdint>
#include <utility>
#include <string>

namespace fap {

struct SourceLocation {
  SourceLocation() = default;
  SourceLocation(std::string fileName, int lineNumber, int columnNumber)
      : file(std::move(fileName)), line(lineNumber), column(columnNumber) {}

  std::string file;
  int line = 0;
  int column = 0;
};

struct AllocationRecord {
  AllocationKind kind = AllocationKind::Unknown;
  std::string operationName;
  std::string valueName;
  SourceLocation location;
  std::uint64_t estimatedBytes = 0;
  bool hasStaticSize = false;
  bool hasMatchingFree = false;
  bool hasRuntimeShape = false;
  bool hasOnlyLocalUses = true;
  bool hasPotentialEscape = false;
  bool requiresCopyInOutCorrectness = false;
  bool hasOverlapRisk = false;
  std::string note;
};

} // namespace fap
