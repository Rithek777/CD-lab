#pragma once

namespace fap {

enum class AllocationKind {
  Unknown,
  ArrayExpressionTemporary,
  ArrayValuedFunctionResult,
  AllocatableAssignmentReallocation,
  CopyInCopyOutTemporary
};

const char *toString(AllocationKind kind);

} // namespace fap
