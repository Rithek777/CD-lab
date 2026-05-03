#pragma once

#include "FAP/AllocationRecord.h"

#include <string>
#include <vector>

namespace fap {

class AnalysisDriver {
public:
  std::vector<AllocationRecord> analyzeFile(const std::string &path) const;
};

} // namespace fap

