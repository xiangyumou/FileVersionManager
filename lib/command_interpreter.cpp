/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef COMMAND_INTERPRETER_CPP
#define COMMAND_INTERPRETER_CPP

#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/ICommandInterpreter.h"
#include "fvm/repositories/ICommandRepository.h"
#include "saver.cpp"
#include "logger.cpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>

#define NO_COMMAND 0x3f3f3f3fULL

class CommandInterpreter : public fvm::interfaces::ICommandInterpreter {
    // 从identifier hash映射到pid
    std::map<unsigned long long, unsigned long long> mp;
    fvm::repositories::ICommandRepository& repository_;
    fvm::interfaces::ILogger& logger_;

    static unsigned long long get_hash(std::string s);
    // bool pid_exist(unsigned long long pid);
    bool identifier_exist(unsigned long long iid);
    std::string escape(char ch);
    std::vector<std::string> separator(std::string &s);
    bool load();
    bool save();
public:
    CommandInterpreter(fvm::interfaces::ILogger& logger,
                       fvm::repositories::ICommandRepository& repository);
    ~CommandInterpreter();

    // Lifecycle management (for testability)
    bool initialize() override;  // Load data from repository
    bool shutdown() override;    // Save data to repository

    bool FIRST_START = false;
    bool add_identifier(const std::string& identifier, unsigned long long pid) override;
    bool delete_identifier(const std::string& identifier) override;
    std::pair<unsigned long long, std::vector<std::string>> get_command() override;
    bool clear_data() override;

    bool is_first_start() const override { return FIRST_START; }
};



                        /* class Terminal */

unsigned long long CommandInterpreter::get_hash(std::string s) {
    unsigned long long hash = 0, seed = 13331;
    for (auto &ch : s) {
        hash = hash * seed + ch;
    }
    return hash;
}

// bool CommandInterpreter::pid_exist(unsigned long long pid) {
//     return pid_set.count(pid);
// }

bool CommandInterpreter::identifier_exist(unsigned long long iid) {
    return mp.count(iid);
}

std::vector<std::string> CommandInterpreter::separator(std::string &s) {
    std::vector<std::string> res;
    std::string tmp;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] != ' ') tmp.push_back(s[i]);
        if (s[i] == ' ' || i == s.size() - 1) {
            if (!tmp.empty()) {
                res.push_back(tmp);
                tmp.clear();
            }
        }
    }
    return res;
}

std::string CommandInterpreter::escape(char ch) {
    static std::vector<std::pair<char, std::string>> fr_to({
        {'s', " "},
        {'t', "\t"},
        {'\\', "\\"}
    });
    for (auto &it : fr_to) {
        if (it.first == ch) return it.second;
    }
    return "";
}

bool CommandInterpreter::save() {
    return repository_.save(mp);
}

bool CommandInterpreter::load() {
    bool result = repository_.load(mp);
    if (mp.empty()) {
        FIRST_START = true;
    }
    return result;
}

CommandInterpreter::CommandInterpreter(fvm::interfaces::ILogger& logger,
                                     fvm::repositories::ICommandRepository& repository)
    : repository_(repository), logger_(logger) {
    // Constructor no longer loads data - use initialize() instead
    FIRST_START = false;
}

CommandInterpreter::~CommandInterpreter() {
    // Destructor no longer saves data - use shutdown() instead
}

bool CommandInterpreter::initialize() {
    // Load data from repository
    bool result = load();
    if (mp.empty()) {
        FIRST_START = true;
    }
    return result;
}

bool CommandInterpreter::shutdown() {
    // Save data to repository
    return save();
}

bool CommandInterpreter::add_identifier(const std::string& identifier, unsigned long long pid) {
    unsigned long long identifier_hash = get_hash(identifier);
    if (identifier_exist(identifier_hash)) {
        logger_.log("Identifier " + identifier + " already exists. Please delete the original to add a new one.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    mp[identifier_hash] = pid;
    return true;
}

bool CommandInterpreter::delete_identifier(const std::string& identifier) {
    unsigned long long identifier_hash = get_hash(identifier);
    if (!identifier_exist(identifier_hash)) {
        logger_.log("Identifier " + identifier + " does not exist.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    unsigned long long pid = mp[identifier_hash];
    // pid_set.erase(pid_set.find(pid));
    mp.erase(mp.find(identifier_hash));
    return true;
}

/**
 * Commands are separated by Spaces. Backslashes can be used to escape Spaces.
 */
std::pair<unsigned long long, std::vector<std::string>> CommandInterpreter::get_command() {
    std::string cmd;
    std::getline(std::cin, cmd);
    std::vector<std::string> separated_cmd = separator(cmd);
    std::vector<std::string> escaped_cmd(separated_cmd.size());
    for (int i = 0; i < separated_cmd.size(); i++) {
        std::string &cmd = separated_cmd[i];
        for (int j = 0; j < cmd.size(); j++) {
            if (cmd[j] != '\\') {
                escaped_cmd[i].push_back(cmd[j]);
            } else if (j == cmd.size() - 1) {
                break;
            } else {
                escaped_cmd[i] += escape(cmd[j + 1]);
                j ++;
            }
        }
    }
    if (escaped_cmd.empty()) {
        return std::make_pair(NO_COMMAND, std::vector<std::string>());
    }
    unsigned long long identifier_hash = get_hash(escaped_cmd.front());
    if (!identifier_exist(identifier_hash)) {
        logger_.log("Command not found: " + escaped_cmd.front(), fvm::interfaces::LogLevel::WARNING, __LINE__);
        return std::make_pair(NO_COMMAND, escaped_cmd);
    } else {
        escaped_cmd.erase(escaped_cmd.begin());
        return std::make_pair(mp[identifier_hash], escaped_cmd);
    }
}

bool CommandInterpreter::clear_data() {
    mp.clear();
    // pid_set.clear();
    return true;
}

#endif