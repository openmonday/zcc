#include "zcc/Basic/SourceFile.h"

#include <utility>

namespace zcc {

SourceFile::SourceFile(std::string text, std::string name)
    : text_(std::move(text)), name_(std::move(name)) {}

SourceFile SourceFile::readAll(std::istream& in, std::string name) {
  std::string text;
  char chunk[4096]; // a read size, not a limit: the string grows past it
  for (;;) {
    in.read(chunk, sizeof chunk);
    text.append(chunk, static_cast<std::size_t>(in.gcount()));
    if (in.gcount() == 0) break;
  }
  return SourceFile(std::move(text), std::move(name));
}

std::string_view SourceFile::slice(std::uint32_t offset,
                                   std::uint32_t length) const {
  return std::string_view(text_).substr(offset, length);
}

} // namespace zcc
