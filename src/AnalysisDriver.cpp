#include "FAP/AnalysisDriver.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fap {
namespace {

std::string trim(const std::string &text) {
  const auto first = std::find_if_not(text.begin(), text.end(),
                                      [](unsigned char ch) {
                                        return std::isspace(ch) != 0;
                                      });
  const auto last = std::find_if_not(text.rbegin(), text.rend(),
                                     [](unsigned char ch) {
                                       return std::isspace(ch) != 0;
                                     })
                        .base();

  if (first >= last) {
    return {};
  }

  return std::string(first, last);
}

std::uint64_t parseInteger(const std::string &text) {
  std::uint64_t value = 0;
  for (const char ch : text) {
    if (!std::isdigit(static_cast<unsigned char>(ch))) {
      break;
    }
    value = value * 10 + static_cast<std::uint64_t>(ch - '0');
  }
  return value;
}

std::uint64_t elementSizeBytes(const std::string &typeName) {
  if (typeName == "f64" || typeName == "i64" || typeName == "index") {
    return 8;
  }
  if (typeName == "f32" || typeName == "i32") {
    return 4;
  }
  if (typeName == "f16" || typeName == "i16") {
    return 2;
  }
  if (typeName == "i8" || typeName == "ui8") {
    return 1;
  }

  return 0;
}

std::uint64_t estimateStaticBytes(const std::string &line) {
  static const std::regex arrayTypePattern(
      R"(!fir\.array<([0-9x]+)x([a-zA-Z0-9_]+)>)");
  std::smatch match;

  if (!std::regex_search(line, match, arrayTypePattern)) {
    return 0;
  }

  const std::string shape = match[1].str();
  const std::string elementType = match[2].str();
  const std::uint64_t elementBytes = elementSizeBytes(elementType);
  if (elementBytes == 0) {
    return 0;
  }

  std::uint64_t elements = 1;
  std::stringstream dims(shape);
  std::string dim;
  while (std::getline(dims, dim, 'x')) {
    const std::uint64_t value = parseInteger(dim);
    if (value == 0) {
      return 0;
    }
    elements *= value;
  }

  return elements * elementBytes;
}

SourceLocation sourceLocationFor(const std::string &path, const std::string &line,
                                 int fallbackLine) {
  static const std::regex locPattern(
      R"MLIRLOC(loc\("([^"]+)":([0-9]+):([0-9]+)\))MLIRLOC");
  std::smatch match;

  if (std::regex_search(line, match, locPattern)) {
    return SourceLocation{match[1].str(), static_cast<int>(parseInteger(match[2])),
                          static_cast<int>(parseInteger(match[3]))};
  }

  return SourceLocation{path, fallbackLine, 1};
}

AllocationKind classifyAllocation(const std::vector<std::string> &context,
                                  const std::string &line) {
  std::string joined;
  for (const auto &entry : context) {
    joined += entry;
    joined += '\n';
  }
  joined += line;

  if (joined.find("hlfir.assign") != std::string::npos ||
      joined.find("realloc") != std::string::npos ||
      joined.find("allocatable") != std::string::npos) {
    return AllocationKind::AllocatableAssignmentReallocation;
  }
  if (joined.find("hlfir.elemental") != std::string::npos ||
      joined.find("hlfir.expr") != std::string::npos ||
      joined.find("array temporary") != std::string::npos) {
    return AllocationKind::ArrayExpressionTemporary;
  }
  if (joined.find("fir.call") != std::string::npos ||
      joined.find("function result") != std::string::npos) {
    return AllocationKind::ArrayValuedFunctionResult;
  }
  if (joined.find("fir.slice") != std::string::npos ||
      joined.find("copy-in") != std::string::npos ||
      joined.find("copy-out") != std::string::npos ||
      joined.find("section") != std::string::npos) {
    return AllocationKind::CopyInCopyOutTemporary;
  }

  return AllocationKind::Unknown;
}

std::string makeNote(bool hasStaticSize, bool hasMatchingFree) {
  std::string note;
  if (!hasStaticSize) {
    note += "static allocation size not visible";
  }
  if (!hasMatchingFree) {
    if (!note.empty()) {
      note += "; ";
    }
    note += "no matching fir.freemem found";
  }
  return note;
}

bool hasRuntimeShape(const std::string &line) {
  return line.find("!fir.array<?") != std::string::npos ||
         line.find("!fir.array<*") != std::string::npos ||
         line.find("typeparams") != std::string::npos;
}

bool hasCopyInOutMarker(const std::string &line) {
  return line.find("fir.copy_in") != std::string::npos ||
         line.find("fir.copy_out") != std::string::npos ||
         line.find("copy-in") != std::string::npos ||
         line.find("copy-out") != std::string::npos;
}

bool hasOverlapMarker(const std::string &line) {
  return line.find("overlap") != std::string::npos ||
         line.find("fir.slice") != std::string::npos ||
         line.find("section") != std::string::npos;
}

bool lineUsesValue(const std::string &line, const std::string &valueName) {
  return line.find(valueName) != std::string::npos;
}

bool useMayEscape(const std::string &line) {
  return line.find("fir.call") != std::string::npos ||
         line.find("fir.embox") != std::string::npos ||
         line.find("fir.rebox") != std::string::npos ||
         line.find("fir.store") != std::string::npos ||
         line.find("hlfir.declare") != std::string::npos ||
         line.find("return ") != std::string::npos;
}

} // namespace

std::vector<AllocationRecord>
AnalysisDriver::analyzeFile(const std::string &path) const {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("unable to open input file: " + path);
  }

  static const std::regex allocPattern(
      R"((%[A-Za-z0-9_.$-]+)\s*=\s*fir\.allocmem\b)");
  static const std::regex freePattern(R"(fir\.freemem\s+(%[A-Za-z0-9_.$-]+))");

  std::vector<AllocationRecord> records;
  std::vector<std::string> context;
  std::string line;
  int lineNumber = 0;

  while (std::getline(input, line)) {
    ++lineNumber;
    const std::string stripped = trim(line);

    for (auto &record : records) {
      if (!record.valueName.empty() && lineUsesValue(stripped, record.valueName) &&
          stripped.find("fir.freemem") == std::string::npos &&
          stripped.find("fir.allocmem") == std::string::npos) {
        if (useMayEscape(stripped)) {
          record.hasPotentialEscape = true;
          record.hasOnlyLocalUses = false;
        }
        if (hasCopyInOutMarker(stripped)) {
          record.requiresCopyInOutCorrectness = true;
        }
        if (hasOverlapMarker(stripped)) {
          record.hasOverlapRisk = true;
        }
      }
    }

    std::smatch allocMatch;
    if (std::regex_search(stripped, allocMatch, allocPattern)) {
      AllocationRecord record;
      record.kind = classifyAllocation(context, stripped);
      record.operationName = "fir.allocmem";
      record.valueName = allocMatch[1].str();
      record.irText = stripped;
      record.location = sourceLocationFor(path, stripped, lineNumber);
      record.estimatedBytes = estimateStaticBytes(stripped);
      record.hasStaticSize = record.estimatedBytes > 0;
      record.hasRuntimeShape = hasRuntimeShape(stripped);
      record.requiresCopyInOutCorrectness = hasCopyInOutMarker(stripped);
      record.hasOverlapRisk = hasOverlapMarker(stripped);
      records.push_back(record);
    }

    std::smatch freeMatch;
    if (std::regex_search(stripped, freeMatch, freePattern)) {
      const std::string freedValue = freeMatch[1].str();
      for (auto &record : records) {
        if (record.valueName == freedValue) {
          record.hasMatchingFree = true;
        }
      }
    }

    if (!stripped.empty()) {
      context.push_back(stripped);
      if (context.size() > 5) {
        context.erase(context.begin());
      }
    }
  }

  for (auto &record : records) {
    record.note = makeNote(record.hasStaticSize, record.hasMatchingFree);
  }

  return records;
}

} // namespace fap
