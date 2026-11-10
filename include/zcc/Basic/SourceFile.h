#pragma once

#include <cstdint>
#include <istream>
#include <string>
#include <string_view>

namespace zcc {

// Owns the whole source text. Every Token is a span into this buffer, so one
// SourceFile must outlive every token produced from it.
class SourceFile {
public:
  explicit SourceFile(std::string text, std::string name = "<stdin>");

  // Reads the stream to the end, growing as needed. No size limit.
  static SourceFile readAll(std::istream& in, std::string name = "<stdin>");

  std::string_view text() const { return text_; }
  std::string_view slice(std::uint32_t offset, std::uint32_t length) const;
  const std::string& name() const { return name_; }

private:
  std::string text_;
  std::string name_;
};

} // namespace zcc
