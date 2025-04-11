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

vector<string> splitBySpace(string input) {
    vector<string> result;
    stringstream ss(input);
    string item;

    while (ss >> item) {
        result.push_back(item);
    }

    return result;
}

vector<command> parseCommands(string input) {
    vector<string> parts = splitBySpace(input);
    vector<command> commands;
    command cmd;

    for (auto it = parts.begin(); it != parts.end(); it++) {
        string part = *it;
        if (part == ">") {
            string file = *(++it);
            cmd.setFileRedirect(file);
        } else if (part == "|") {
            cmd.setPipe(PIPE_NORMAL, 1);
        } else if (part.size() > 1 && part[0] == '|' && isdigit(part[1])) {
            cmd.setPipe(PIPE_NUMBERED, stoi(part.substr(1)));
        } else if (part.size() > 1 && part[0] == '!' && isdigit(part[1])) {
            cmd.setPipe(PIPE_NUMBERED_ERR, stoi(part.substr(1)));
        } else {
            cmd.addArg(part);
            continue;
        }
        commands.push_back(cmd);
        cmd = command();
    }

    if (!cmd.args.empty()) {
        commands.push_back(cmd);
    }

    return commands;
}

command_type getCommandType(string command) {
    if (command == COMMAND_PRINT_ENV) {
        return COMMAND_TYPE_PRINT_ENV;
    } else if (command == COMMAND_SET_ENV) {
        return COMMAND_TYPE_SET_ENV;
    }
    return COMMAND_TYPE_ELSE;
}

void printCommands(vector<command> commands) {
    for (auto command : commands) {
        cout << "cmd: ";
        for (auto arg : command.args) {
            cout << arg << " ";
        }
        cout << "pipe: " << command.pipe << " pipe_num: " << command.pipeNum << "\n";
        cout << "isFileRedirect: " << command.fileRedirect << " outputFile: " << command.outputFile << "\n";
        cout << "--------------------------------\n";
    }
}

#endif
