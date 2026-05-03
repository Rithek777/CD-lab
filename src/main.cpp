#include "FAP/AnalysisDriver.h"
#include "FAP/AllocationKind.h"

#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

void printUsage(const char *programName) {
  std::cout << "Usage: flang-implicit-alloc-profiler <input.mlir>\n\n"
            << "Flang Implicit Allocation Profiler and Optimizer\n"
            << "Scans textual Flang HLFIR/FIR MLIR for implicit heap allocations.\n\n"
            << "Detection targets:\n"
            << "  - fir.allocmem with matching fir.freemem\n"
            << "  - hlfir.expr and hlfir.elemental temporaries\n"
            << "  - hlfir.assign allocatable reallocation\n"
            << "  - copy-in/copy-out array section temporaries\n";

  (void)programName;
}

double bytesToMiB(std::uint64_t bytes) {
  return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

} // namespace

int main(int argc, char **argv) {
  if (argc != 2 || std::string(argv[1]) == "--help" ||
      std::string(argv[1]) == "-h") {
    printUsage(argv[0]);
    return argc == 2 ? 0 : 1;
  }

  try {
    fap::AnalysisDriver driver;
    const auto records = driver.analyzeFile(argv[1]);

    std::cout << "Flang Implicit Allocation Report\n"
              << "Input: " << argv[1] << '\n'
              << "Allocations detected: " << records.size() << "\n\n";

    if (records.empty()) {
      std::cout << "No fir.allocmem operations found.\n";
      return 0;
    }

    for (const auto &record : records) {
      std::cout << "line " << record.location.line
                << ": implicit heap allocation detected";

      if (record.hasStaticSize) {
        std::cout << ", estimated size = " << std::fixed
                  << std::setprecision(2) << bytesToMiB(record.estimatedBytes)
                  << " MB";
      } else {
        std::cout << ", estimated size = unknown";
      }

      std::cout << ", reason = " << fap::toString(record.kind)
                << ", value = " << record.valueName
                << ", matching free = "
                << (record.hasMatchingFree ? "yes" : "no");

      if (!record.location.file.empty()) {
        std::cout << ", source = " << record.location.file << ':'
                  << record.location.line << ':' << record.location.column;
      }

      if (!record.note.empty()) {
        std::cout << ", note = " << record.note;
      }

      std::cout << '\n';
    }
  } catch (const std::exception &error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
