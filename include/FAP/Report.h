#pragma once

#include "FAP/AllocationClassifier.h"
#include "FAP/AllocationRecord.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace fap {

enum class ReportFormat {
  Text,
  Json
};

struct ReportOptions {
  ReportFormat format = ReportFormat::Text;
  bool showIr = false;
  double thresholdMb = 0.0;
};

class ReportWriter {
public:
  void write(std::ostream &output, const std::string &inputPath,
             const std::vector<AllocationRecord> &records,
             const ReportOptions &options) const;

private:
  AllocationClassifier classifier;
};

} // namespace fap

