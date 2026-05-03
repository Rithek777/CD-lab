#include "FAP/AnalysisDriver.h"

#include <fstream>
#include <stdexcept>

namespace fap {

std::vector<AllocationRecord>
AnalysisDriver::analyzeFile(const std::string &path) const {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("unable to open input file: " + path);
  }

  // Skeleton only: future work will parse MLIR and walk HLFIR/FIR operations.
  return {};
}

} // namespace fap

