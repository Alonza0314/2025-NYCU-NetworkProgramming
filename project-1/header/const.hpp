#ifndef CONST_HPP
#define CONST_HPP

#include "header.hpp"

#define nil nullptr
#define error char*

#define NOT_FOUND -1

error ERROR_INVALID_NUMBER_OF_ARGUMENTS = (char*)"Invalid number of arguments.\n";
error ERROR_FORK_FAILED = (char*)"Fork failed.\n";
error ERROR_WAITPID_FAILED = (char*)"Waitpid failed.\n";
error ERROR_PIPE_FAILED = (char*)"Pipe creation failed.\n";
error ERROR_OPEN_FILE_FAILED = (char*)"Cannot open file.\n";

const string COMMAND_PRINT_ENV = "printenv";
const string COMMAND_SET_ENV = "setenv";
const string COMMAND_EXIT = "exit";

enum command_type {
    COMMAND_TYPE_PRINT_ENV,
    COMMAND_TYPE_SET_ENV,
    COMMAND_TYPE_ELSE
};

enum pipe_type {
    PIPE_NONE,
    PIPE_NORMAL,
    PIPE_NUMBERED,
    PIPE_NUMBERED_ERR
};

#endif
