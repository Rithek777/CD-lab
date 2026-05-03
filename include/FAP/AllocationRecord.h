#pragma once

#include "FAP/AllocationKind.h"

#include <string>

namespace fap {

struct SourceLocation {
  std::string file;
  int line = 0;
  int column = 0;
};

struct AllocationRecord {
  AllocationKind kind = AllocationKind::Unknown;
  std::string operationName;
  SourceLocation location;
  std::string note;
};

} // namespace fap

