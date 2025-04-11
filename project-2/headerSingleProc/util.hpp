#ifndef UTIL_HPP
#define UTIL_HPP

#include "header.hpp"
#include "const.hpp"
#include "model.hpp"

bool isInputEOF(string &input) {
    if (!getline(cin, input)) {
        cout << "\n";
        return true;
    }
    return false;
}

bool isExitCommand(const string &input) {
    stringstream ss(input);
    string command;
    ss >> command;
    return !command.empty() && command == COMMAND_EXIT;
}

bool getAndCheckIsExitAndEof(string &input) {
    if (isInputEOF(input)) {
        return true;
    }
    return isExitCommand(input);
}

bool isNumber(const string &str) {
    return !str.empty() && all_of(str.begin(), str.end(), ::isdigit);
}

#endif
