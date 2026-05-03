#include "FAP/AnalysisDriver.h"
#include "FAP/Report.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void printUsage(const char *programName) {
  std::cout << "Usage: flang-implicit-alloc-profiler [options] <input.mlir>\n\n"
            << "Flang Implicit Allocation Profiler and Optimizer\n"
            << "Scans textual Flang HLFIR/FIR MLIR for implicit heap allocations.\n\n"
            << "Options:\n"
            << "  --format=text       Print a human-readable report (default)\n"
            << "  --format=json       Print a JSON report\n"
            << "  --show-ir           Include the matched fir.allocmem line\n"
            << "  --threshold-mb=N    Only report allocations at least N MB; unknown sizes remain visible\n\n"
            << "Detection targets:\n"
            << "  - fir.allocmem with matching fir.freemem\n"
            << "  - hlfir.expr and hlfir.elemental temporaries\n"
            << "  - hlfir.assign allocatable reallocation\n"
            << "  - copy-in/copy-out array section temporaries\n";

  (void)programName;
}

struct CliOptions {
  fap::ReportOptions report;
  std::string inputPath;
};

bool startsWith(const std::string &text, const std::string &prefix) {
  return text.size() >= prefix.size() &&
         text.compare(0, prefix.size(), prefix) == 0;
}

CliOptions parseArgs(int argc, char **argv) {
  CliOptions options;

  for (int index = 1; index < argc; ++index) {
    const std::string arg = argv[index];

    if (arg == "--help" || arg == "-h") {
      printUsage(argv[0]);
      std::exit(0);
    }

    if (arg == "--show-ir") {
      options.report.showIr = true;
      continue;
    }

    if (startsWith(arg, "--format=")) {
      const std::string value = arg.substr(std::string("--format=").size());
      if (value == "text") {
        options.report.format = fap::ReportFormat::Text;
      } else if (value == "json") {
        options.report.format = fap::ReportFormat::Json;
      } else {
        throw std::runtime_error("unknown report format: " + value);
      }
      continue;
    }

    if (startsWith(arg, "--threshold-mb=")) {
      const std::string value =
          arg.substr(std::string("--threshold-mb=").size());
      options.report.thresholdMb = std::stod(value);
      if (options.report.thresholdMb < 0.0) {
        throw std::runtime_error("--threshold-mb must be non-negative");
      }
      continue;
    }

    if (!arg.empty() && arg[0] == '-') {
      throw std::runtime_error("unknown option: " + arg);
    }

    if (!options.inputPath.empty()) {
      throw std::runtime_error("only one input MLIR file can be provided");
    }
    options.inputPath = arg;
  }

  if (options.inputPath.empty()) {
    throw std::runtime_error("missing input MLIR file");
  }

  return options;
}

} // namespace

int main(int argc, char **argv) {
  try {
    const auto options = parseArgs(argc, argv);
    fap::AnalysisDriver driver;
    fap::ReportWriter writer;
    const auto records = driver.analyzeFile(options.inputPath);
    writer.write(std::cout, options.inputPath, records, options.report);
  } catch (const std::exception &error) {
    std::cerr << "error: " << error.what() << '\n';
    std::cerr << "run with --help for usage\n";
    return 1;
  }

  return 0;
}
