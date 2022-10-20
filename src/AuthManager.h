#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "llvm/Support/Error.h"

extern "C" {
#include <mosquitto_plugin.h>
}

#include "UserPassEntry.h"

class AuthManager {
private:
  std::unordered_map<std::string, std::string> UserPassEntries;

public:
  AuthManager() = default;

  llvm::Error loadConfig(struct mosquitto_opt *opts, int opt_count) {
    std::string_view UsersFile;
    for (int i = 0; i < opt_count; i++) {
      if (std::string_view(opts[i].key) == "users_file") {
        UsersFile = opts[i].value;
        break;
      }
    }
    if (UsersFile.empty()) {
      return llvm::createStringError(llvm::inconvertibleErrorCode(), "users_file option not found");
    }
    auto EntriesOrErr = UserPassEntry::loadFromFile(std::string(UsersFile));
    if (std::error_code ec = EntriesOrErr.getError()) {
      return llvm::createStringError(ec, "failed to load users file");
    }
    UserPassEntries = UserPassEntry::asMap(EntriesOrErr.get());
    return llvm::Error::success();
  }

  void clearConfig() { UserPassEntries.clear(); }

  bool hasUser(const char *username) const {
    if (username == nullptr) {
      return false;
    }
    return UserPassEntries.count(username) > 0;
  }

  bool checkUser(const char *username, const char *password) {
    if (username == nullptr || password == nullptr) {
      return false;
    }
    auto it = UserPassEntries.find(username);
    if (it == UserPassEntries.end()) {
      return false;
    }
    return it->second == password;
  }
};
