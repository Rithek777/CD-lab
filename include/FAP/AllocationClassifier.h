#pragma once

#include "FAP/AllocationInfo.h"
#include "FAP/AllocationRecord.h"

namespace fap {

class AllocationClassifier {
public:
  AllocationInfo classify(const AllocationRecord &record) const;
};

} // namespace fap

