#include "FAP/AllocationKind.h"

namespace fap {

std::string_view toString(AllocationKind kind) {
  switch (kind) {
  case AllocationKind::ArrayExpressionTemporary:
    return "array expression temporary";
  case AllocationKind::ArrayValuedFunctionResult:
    return "array-valued function result";
  case AllocationKind::AllocatableAssignmentReallocation:
    return "allocatable assignment reallocation";
  case AllocationKind::CopyInCopyOutTemporary:
    return "copy-in/copy-out temporary";
  case AllocationKind::Unknown:
    return "unknown";
  }

  return "unknown";
}

} // namespace fap

