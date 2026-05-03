#include "FAP/AllocationClassifier.h"

namespace fap {

const char *toString(AllocationClassification classification) {
  switch (classification) {
  case AllocationClassification::ProvablyUnnecessary:
    return "PROVABLY_UNNECESSARY";
  case AllocationClassification::PossiblyUnnecessary:
    return "POSSIBLY_UNNECESSARY";
  case AllocationClassification::Necessary:
    return "NECESSARY";
  }

  return "POSSIBLY_UNNECESSARY";
}

AllocationInfo AllocationClassifier::classify(const AllocationRecord &record) const {
  AllocationInfo info;

  if (record.kind == AllocationKind::AllocatableAssignmentReallocation) {
    info.classification = AllocationClassification::Necessary;
    info.reason =
        "allocatable assignment may require reallocation to satisfy shape or allocation state";
    info.confidence = 0.86;
    info.suggestedFix =
        "preallocate the allocatable with the final shape before assignment when possible";
    return info;
  }

  if (record.kind == AllocationKind::CopyInCopyOutTemporary ||
      record.requiresCopyInOutCorrectness || record.hasOverlapRisk) {
    info.classification = AllocationClassification::Necessary;
    info.reason =
        "temporary appears needed for non-contiguous section or copy-in/copy-out correctness";
    info.confidence = 0.82;
    info.suggestedFix =
        "pass a contiguous array, add a contiguous interface when valid, or avoid strided sections";
    return info;
  }

  if (record.hasPotentialEscape || !record.hasMatchingFree) {
    info.classification = AllocationClassification::PossiblyUnnecessary;
    info.reason =
        "allocation may escape through a pointer, descriptor, call, or missing local freemem";
    info.confidence = 0.48;
    info.suggestedFix =
        "inspect uses of the allocated value and keep the temporary local if it is safe";
    return info;
  }

  if (!record.hasStaticSize || record.hasRuntimeShape) {
    info.classification = AllocationClassification::PossiblyUnnecessary;
    info.reason =
        "shape or runtime size is not statically visible, so avoidability is uncertain";
    info.confidence = 0.58;
    info.suggestedFix =
        "provide explicit bounds or preallocated storage when the shape is known by the programmer";
    return info;
  }

  if (record.kind == AllocationKind::ArrayValuedFunctionResult) {
    info.classification = AllocationClassification::PossiblyUnnecessary;
    info.reason =
        "array-valued function result temporary may be avoidable depending on caller/result lowering";
    info.confidence = 0.60;
    info.suggestedFix =
        "consider a subroutine with an explicit output argument for large array results";
    return info;
  }

  if (record.kind == AllocationKind::ArrayExpressionTemporary &&
      record.hasOnlyLocalUses && record.hasMatchingFree && !record.hasPotentialEscape) {
    info.classification = AllocationClassification::ProvablyUnnecessary;
    info.reason =
        "temporary is local, has static shape, does not appear to escape, and is freed locally";
    info.confidence = 0.78;
    info.suggestedFix =
        "replace the whole-array expression with an explicit loop or stack-sized local buffer";
    return info;
  }

  info.classification = AllocationClassification::PossiblyUnnecessary;
  info.reason = "allocation is detected, but the textual scanner lacks enough context";
  info.confidence = 0.50;
  info.suggestedFix =
      "review nearby HLFIR/FIR operations and reduce temporary-producing array syntax if practical";
  return info;
}

} // namespace fap

