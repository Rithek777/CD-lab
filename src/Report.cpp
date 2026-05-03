#include "FAP/Report.h"

#include "FAP/AllocationKind.h"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace fap {
namespace {

double bytesToMb(std::uint64_t bytes) {
  return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

bool passesThreshold(const AllocationRecord &record, double thresholdMb) {
  if (thresholdMb <= 0.0) {
    return true;
  }

  if (!record.hasStaticSize) {
    return true;
  }

  return bytesToMb(record.estimatedBytes) >= thresholdMb;
}

std::string jsonEscape(const std::string &text) {
  std::ostringstream escaped;
  for (const char ch : text) {
    switch (ch) {
    case '\\':
      escaped << "\\\\";
      break;
    case '"':
      escaped << "\\\"";
      break;
    case '\n':
      escaped << "\\n";
      break;
    case '\r':
      escaped << "\\r";
      break;
    case '\t':
      escaped << "\\t";
      break;
    default:
      escaped << ch;
      break;
    }
  }
  return escaped.str();
}

void writeTextReport(std::ostream &output, const std::string &inputPath,
                     const std::vector<AllocationRecord> &records,
                     const ReportOptions &options,
                     const AllocationClassifier &classifier) {
  std::size_t shown = 0;
  for (const auto &record : records) {
    if (passesThreshold(record, options.thresholdMb)) {
      ++shown;
    }
  }

  output << "Flang Implicit Allocation Report\n"
         << "Input: " << inputPath << '\n'
         << "Allocations reported: " << shown << '\n'
         << "Threshold: " << std::fixed << std::setprecision(2)
         << options.thresholdMb << " MB\n\n";

  if (shown == 0) {
    output << "No allocations met the reporting threshold.\n";
    return;
  }

  for (const auto &record : records) {
    if (!passesThreshold(record, options.thresholdMb)) {
      continue;
    }

    const auto info = classifier.classify(record);
    const double estimatedMb = record.hasStaticSize ? bytesToMb(record.estimatedBytes) : 0.0;

    output << "line " << record.location.line << ": "
           << fap::toString(record.kind) << " generates ";

    if (record.hasStaticSize) {
      output << std::fixed << std::setprecision(2) << estimatedMb
             << " MB temporary array allocation\n";
    } else {
      output << "an unknown-size temporary array allocation\n";
    }

    output << "source file: " << record.location.file << '\n'
           << "source line: " << record.location.line << '\n'
           << "source column: " << record.location.column << '\n'
           << "IR operation: " << record.operationName << '\n'
           << "estimated bytes: " << record.estimatedBytes << '\n'
           << "estimated MB: ";

    if (record.hasStaticSize) {
      output << std::fixed << std::setprecision(2) << estimatedMb << '\n';
    } else {
      output << "unknown\n";
    }

    output << "classification: " << fap::toString(info.classification) << '\n'
           << "reason: " << info.reason << '\n'
           << "confidence: " << std::fixed << std::setprecision(2)
           << info.confidence << '\n'
           << "suggestion: " << info.suggestedFix << '\n';

    if (options.showIr) {
      output << "ir: " << record.irText << '\n';
    }

    output << '\n';
  }
}

void writeJsonReport(std::ostream &output, const std::string &inputPath,
                     const std::vector<AllocationRecord> &records,
                     const ReportOptions &options,
                     const AllocationClassifier &classifier) {
  output << "{\n"
         << "  \"input\": \"" << jsonEscape(inputPath) << "\",\n"
         << "  \"format\": \"json\",\n"
         << "  \"threshold_mb\": " << std::fixed << std::setprecision(2)
         << options.thresholdMb << ",\n"
         << "  \"allocations\": [\n";

  bool first = true;
  for (const auto &record : records) {
    if (!passesThreshold(record, options.thresholdMb)) {
      continue;
    }

    const auto info = classifier.classify(record);
    const double estimatedMb = record.hasStaticSize ? bytesToMb(record.estimatedBytes) : 0.0;

    if (!first) {
      output << ",\n";
    }
    first = false;

    output << "    {\n"
           << "      \"source_file\": \"" << jsonEscape(record.location.file)
           << "\",\n"
           << "      \"source_line\": " << record.location.line << ",\n"
           << "      \"source_column\": " << record.location.column << ",\n"
           << "      \"ir_operation_name\": \"" << record.operationName
           << "\",\n"
           << "      \"estimated_bytes\": " << record.estimatedBytes << ",\n"
           << "      \"estimated_mb\": ";

    if (record.hasStaticSize) {
      output << std::fixed << std::setprecision(2) << estimatedMb;
    } else {
      output << "null";
    }

    output << ",\n"
           << "      \"classification\": \""
           << fap::toString(info.classification) << "\",\n"
           << "      \"reason\": \"" << jsonEscape(info.reason) << "\",\n"
           << "      \"suggested_transformation\": \""
           << jsonEscape(info.suggestedFix) << "\",\n"
           << "      \"confidence\": " << std::fixed << std::setprecision(2)
           << info.confidence;

    if (options.showIr) {
      output << ",\n"
             << "      \"ir\": \"" << jsonEscape(record.irText) << "\"\n";
    } else {
      output << '\n';
    }

    output << "    }";
  }

  output << "\n"
         << "  ]\n"
         << "}\n";
}

} // namespace

void ReportWriter::write(std::ostream &output, const std::string &inputPath,
                         const std::vector<AllocationRecord> &records,
                         const ReportOptions &options) const {
  if (options.format == ReportFormat::Json) {
    writeJsonReport(output, inputPath, records, options, classifier);
    return;
  }

  writeTextReport(output, inputPath, records, options, classifier);
}

} // namespace fap

