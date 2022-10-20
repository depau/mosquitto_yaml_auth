#pragma once

#include <vector>
#include <unordered_map>

#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/YAMLTraits.h"

class UserPassEntry {
  friend struct llvm::yaml::MappingTraits<UserPassEntry>;

private:
  std::string Username;
  std::string Password;

public:
  UserPassEntry() = default;
  UserPassEntry(std::string Username, std::string Password) :
    Username(std::move(Username)), Password(std::move(Password)) {}

  [[nodiscard]] const std::string &getUsername() const { return Username; }
  void setUsername(const std::string &username) { Username = username; }
  [[nodiscard]] const std::string &getPassword() const { return Password; }
  void setPassword(const std::string &password) { Password = password; }

  static llvm::ErrorOr<std::vector<UserPassEntry>> loadFromFile(const std::string &filename);

  static std::unordered_map<std::string, std::string> asMap(const std::vector<UserPassEntry> &entries) {
    std::unordered_map<std::string, std::string> map;
    for (const auto &entry : entries) {
      map[entry.getUsername()] = entry.getPassword();
    }
    return map;
  }
};

namespace llvm::yaml {
template<>
struct MappingTraits<UserPassEntry> {
  static void mapping(IO &io, UserPassEntry &entry) {
    io.mapRequired("username", entry.Username);
    io.mapRequired("password", entry.Password);
  }
  static const bool flow = false;
};
} // namespace llvm::yaml

LLVM_YAML_IS_SEQUENCE_VECTOR(UserPassEntry);