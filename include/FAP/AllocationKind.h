#pragma once

#include <string_view>

namespace fap {

enum class AllocationKind {
  Unknown,
  ArrayExpressionTemporary,
  ArrayValuedFunctionResult,
  AllocatableAssignmentReallocation,
  CopyInCopyOutTemporary
};

std::string_view toString(AllocationKind kind);

} // namespace fap

