#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace zcc {

// One entry per distinct problem the compiler can report.
enum class DiagID {
  MultipleDecimalPoints,
  DecimalPointAtEnds,
  LeadingZerosInInteger,
  UnrecognizableCharacter,
};

struct Diagnostic {
  DiagID id;
  std::uint32_t offset;
  std::uint32_t length;
};

// Collects diagnostics in source order. Reporting never stops the lexer;
// the driver decides what to do with the first one.
class DiagnosticEngine {
public:
  void report(DiagID id, std::uint32_t offset, std::uint32_t length) {
    diags_.push_back(Diagnostic{id, offset, length});
  }

  bool empty() const { return diags_.empty(); }
  std::size_t size() const { return diags_.size(); }
  const Diagnostic& first() const { return diags_.front(); }
  const Diagnostic& at(std::size_t i) const { return diags_[i]; }
  void clear() { diags_.clear(); }

private:
  std::vector<Diagnostic> diags_;
};

} // namespace zcc
