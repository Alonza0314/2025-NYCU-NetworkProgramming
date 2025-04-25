#ifndef CONST_HPP
#define CONST_HPP

#include "header.hpp"

#define nil nullptr
#define error char*
#define systemMsg char*
#define socketFd int

#define NOT_FOUND -1

error ERROR_INVALID_NUMBER_OF_ARGUMENTS = (char*)"Invalid number of arguments.\n";
error ERROR_FORK_FAILED = (char*)"Fork failed.\n";
error ERROR_WAITPID_FAILED = (char*)"Waitpid failed.\n";
error ERROR_PIPE_FAILED = (char*)"Pipe creation failed.\n";
error ERROR_OPEN_FILE_FAILED = (char*)"Cannot open file.\n";
error ERROR_SETENV_FAILED = (char*)"Setenv failed.\n";
error ERROR_ID_NOT_NUMBER = (char*)"ID is not a number.\n";
error ERROR_PIPE_BUFFER_FULL = (char*)"Pipe buffer is full.\n";
error ERROR_USER_PIPE_BUFFER_FULL = (char*)"User pipe buffer is full.\n";
error ERROR_OPEN_USER_PIPE_READ_FAILED = (char*)"Open user pipe read failed.\n";
error ERROR_OPEN_USER_PIPE_WRITE_FAILED = (char*)"Open user pipe write failed.\n";

error ERROR_SOCKET_CREATION_FAILED = (char*)"Socket creation failed.\n";
error ERROR_SOCKET_OPTION_SETTING_FAILED = (char*)"Socket option setting failed.\n";
error ERROR_SOCKET_BINDING_FAILED = (char*)"Socket binding failed.\n";
error ERROR_SOCKET_LISTENING_FAILED = (char*)"Socket listening failed.\n";

error ERROR_USER_LOGIN_FAILED_WITH_RUN_OUT_OF_SQN = (char*)"User login failed with run out of sequence number.\n";
error ERROR_USER_LOGOUT_FAILED_WITH_USER_NOT_FOUND = (char*)"User logout failed with user not found.\n";
error ERROR_USER_DISCONNECTED = (char*)"User disconnected.\n";
error ERROR_USER_NOT_FOUND = (char*)"User not found.\n";

error ERROR_SERVER_MESSAGE_SEND_PROMPT_MESSAGE_FAILED = (char*)"Server message send prompt message failed.\n";
error ERROR_SERVER_MESSAGE_SEND_WELCOME_MESSAGE_FAILED = (char*)"Server message send welcome message failed.\n";
error ERROR_SERVER_MESSAGE_BROADCAST_FAILED = (char*)"Server message broadcast failed.\n";
error ERROR_SERVER_MESSAGE_SEND_WHO_TITLE_FAILED = (char*)"Server message send who title failed.\n";
error ERROR_SERVER_MESSAGE_SEND_WHO_USER_FAILED = (char*)"Server message send who user failed.\n";
error ERROR_SERVER_MESSAGE_SEND_TELL_FAILED = (char*)"Server message send tell failed.\n";
error ERROR_SERVER_MESSAGE_NAME_UPDATE_FAILED = (char*)"Server message name update failed.\n";
error ERROR_SERVER_MESSAGE_USER_PIPE_ALREADY_EXISTS_FAILED = (char*)"*** Error: the pipe #%d->#%d already exists. ***\n";
error ERROR_SERVER_MESSAGE_USER_PIPE_NOT_EXISTS_FAILED = (char*)"*** Error: the pipe #%d->#%d does not exist yet. ***\n";
error ERROR_SERVER_MESSAGE_USER_PIPE_USER_NOT_EXIST = (char*)"*** Error: user #%d does not exist yet. ***\n";

error ERROR_SHM_CREATE_FAILED = (char*)"Failed to create shared memory";
error ERROR_SHM_INIT_FAILED = (char*)"Failed to initialize shared memory";
error ERROR_SHM_ATTACH_FAILED = (char*)"Failed to attach shared memory";
error ERROR_SHM_DETACH_FAILED = (char*)"Failed to detach shared memory";
error ERROR_SHM_DELETE_FAILED = (char*)"Failed to delete shared memory";

const string COMMAND_PRINT_ENV = "printenv";
const string COMMAND_SET_ENV = "setenv";
const string COMMAND_EXIT = "exit";

const string SERVER_COMMAND_WHO = "who";
const string SERVER_COMMAND_TELL = "tell";
const string SERVER_COMMAND_YELL = "yell";
const string SERVER_COMMAND_NAME = "name";

const int USER_DEFAULT_SQN = -1;
const string USER_DEFAULT_NAME = "(no name)";
const string USER_DEFAULT_IP = "";
const string USER_DEFAULT_PORT = "";
const string USER_DEFAULT_ENV_KEY = "PATH";
const string USER_DEFAULT_ENV_VALUE = "bin:.";

const string USER_PIPE_DIR = "user_pipe";

const systemMsg SERVER_MESSAGE_PROMPT = (char*)"% ";
const systemMsg SERVER_MESSAGE_WELCOME = (char*)"****************************************\n** Welcome to the information server. **\n****************************************\n";
const systemMsg SERVER_MESSAGE_LOGIN = (char*)"*** User '%s' entered from %s:%s. ***\n";
const systemMsg SERVER_MESSAGE_LOGOUT = (char*)"*** User '%s' left. ***\n";
const systemMsg SERVER_MESSAGE_WHO_TITLE = (char*)"<ID>\t<nickname>\t<IP:port>\t<indicate me>\n";
const systemMsg SERVER_MESSAGE_WHO_USER = (char*)"%d\t%s\t%s:%s\n";
const systemMsg SERVER_MESSAGE_WHO_USER_SELF = (char*)"%d\t%s\t%s:%s\t<-me\n";
const systemMsg SERVER_MESSAGE_TELL_SUCCESS = (char*)"*** %s told you ***: %s\n";
const systemMsg SERVER_MESSAGE_TELL_FAILED = (char*)"*** Error: user #%s does not exist yet. ***\n";
const systemMsg SERVER_MESSAGE_YELL = (char*)"*** %s yelled ***: %s\n";
const systemMsg SERVER_MESSAGE_NAME_UPDATE_SUCCESS = (char*)"*** User from %s:%s is named '%s'. ***\n";
const systemMsg SERVER_MESSAGE_NAME_UPDATE_FAILED = (char*)"*** User '%s' already exists. ***\n";
const systemMsg SERVER_MESSAGE_USER_PIPE_SEND_SUCCESS = (char*)"*** %s (#%d) just piped '%s' to %s (#%d) ***\n";
const systemMsg SERVER_MESSAGE_USER_PIPE_RECV_SUCCESS = (char*)"*** %s (#%d) just received from %s (#%d) by '%s' ***\n";

const int PORT = 7001;
const int BUFFER_SIZE = 1600;
const int MAX_CLIENTS_USERS = 30;

const int CLIENT_DISCONNECTED = -2;
const int CLIENT_ERROR = -1;

const int SHM_ID = 0314;
const int SHM_KEY = 0314;

enum command_type {
    COMMAND_TYPE_PRINT_ENV,
    COMMAND_TYPE_SET_ENV,
    COMMAND_TYPE_SERVER,
    COMMAND_TYPE_ELSE
};

enum pipe_type {
    PIPE_NONE,
    PIPE_NORMAL,
    PIPE_NUMBERED,
    PIPE_NUMBERED_ERR
};

enum server_command_type {
    SERVER_COMMAND_TYPE_WHO,
    SERVER_COMMAND_TYPE_TELL,
    SERVER_COMMAND_TYPE_YELL,
    SERVER_COMMAND_TYPE_NAME,
    SERVER_COMMAND_TYPE_ELSE
};

#endif
