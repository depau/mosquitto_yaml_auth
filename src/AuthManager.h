#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "llvm/Support/Error.h"

extern "C" {
#include <mosquitto.h>
#include <mosquitto_plugin.h>
}

#include "UserPassEntry.h"

class AuthManager {
private:
  std::unordered_map<std::string, std::string> UserPassEntries;
  bool Debug = false;

public:
  void log(std::string_view message) const {
    if (!Debug)
      return;
    mosquitto_log_printf(MOSQ_LOG_INFO, "%s", message.data());
  }

  AuthManager() {
    if (const char *debug = std::getenv("YAML_AUTH_DEBUG")) {
      Debug = std::string_view(debug) == "1";
    }
  }

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
    log("Loading users file: " + std::string(UsersFile));
    auto EntriesOrErr = UserPassEntry::loadFromFile(std::string(UsersFile));
    if (std::error_code ec = EntriesOrErr.getError()) {
      return llvm::createStringError(ec, "failed to load users file");
    }
    UserPassEntries = UserPassEntry::asMap(EntriesOrErr.get());
    return llvm::Error::success();
  }

  void clearConfig() {
    log("Clearing users config");
    UserPassEntries.clear();
  }

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
    log("Checking user: " + std::string(username) + " password: " + std::string(password));
    auto it = UserPassEntries.find(username);
    if (it == UserPassEntries.end()) {
      return false;
    }
    return it->second == password;
  }
};
