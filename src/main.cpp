#include "FAP/AnalysisDriver.h"

#include <exception>
#include <iostream>
#include <string>

namespace {

void printUsage(const char *programName) {
  std::cout << "Usage: " << programName << " <input.mlir>\n\n"
            << "Flang Implicit Allocation Profiler and Optimizer\n"
            << "Skeleton build: analysis is not implemented yet.\n\n"
            << "Planned detection targets:\n"
            << "  - fir.allocmem / fir.freemem\n"
            << "  - hlfir.expr and hlfir.elemental temporaries\n"
            << "  - hlfir.assign allocatable reallocation\n"
            << "  - copy-in/copy-out array section temporaries\n";
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

    std::cout << "Analysis skeleton ready.\n"
              << "Input: " << argv[1] << '\n'
              << "Detected implicit allocations: " << records.size() << '\n'
              << "Full HLFIR/FIR detection will be implemented later.\n";
  } catch (const std::exception &error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }

  return 0;
}

