#include "UserPassEntry.h"

llvm::ErrorOr<std::vector<UserPassEntry>> UserPassEntry::loadFromFile(const std::string &filename) {
  // read file as llvm MemoryBuffer
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> maybeBuffer = llvm::MemoryBuffer::getFile(filename);
  if (std::error_code ec = maybeBuffer.getError()) {
    return ec;
  }
  std::unique_ptr<llvm::MemoryBuffer> buffer = std::move(maybeBuffer.get());

  // parse yaml
  llvm::yaml::Input yamlInput(buffer->getBuffer());
  std::vector<UserPassEntry> entries;
  yamlInput >> entries;

  return entries;
}