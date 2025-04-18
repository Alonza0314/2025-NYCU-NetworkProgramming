#ifndef MODEL_HPP
#define MODEL_HPP

#include "header.hpp"
#include "const.hpp"

struct command {
    vector<string> args;
    string originalInput;

    pipe_type pipe;
    int pipeNum;

    bool fileRedirect;
    string outputFile;

    bool sendPipe;
    bool recvPipe;
    int sendSqn;
    int recvSqn;

    command() : args(), pipe(PIPE_NONE), pipeNum(0), fileRedirect(false), outputFile(""), sendPipe(false), recvPipe(false), sendSqn(NOT_FOUND), recvSqn(NOT_FOUND) {}
    ~command() {
        args.clear();
    }

    void addArg(string arg) {
        this->args.push_back(arg);
    }

    void setPipe(pipe_type pipe, int pipeNum) {
        this->pipe = pipe;
        this->pipeNum = pipeNum;
    }

    void setFileRedirect(string outputFile) {
        this->fileRedirect = true;
        this->outputFile = outputFile;
    }

    void setSendPipe(int sendSqn) {
        this->sendPipe = true;
        this->sendSqn = sendSqn;
    }

    void setRecvPipe(int recvSqn) {
        this->recvPipe = true;
        this->recvSqn = recvSqn;
    }

    bool isPipe() {
        return this->pipe != PIPE_NONE;
    }

    bool isPipeNormal() {
        return this->pipe == PIPE_NORMAL;
    }

    bool isPipeNumbered() {
        return this->pipe == PIPE_NUMBERED;
    }

    bool isPipeNumberedErr() {
        return this->pipe == PIPE_NUMBERED_ERR;
    }

    bool isFileRedirect() {
        return this->fileRedirect;
    }

    bool hasSendPipe() {
        return this->sendPipe;
    }

    bool hasRecvPipe() {
        return this->recvPipe;
    }

    string getOriginalInput() {
        return this->originalInput;
    }

    command_type getCommandType() {
        if (this->args[0] == COMMAND_PRINT_ENV) {
            return COMMAND_TYPE_PRINT_ENV;
        } else if (this->args[0] == COMMAND_SET_ENV) {
            return COMMAND_TYPE_SET_ENV;
        } else if (this->args[0] == SERVER_COMMAND_WHO || this->args[0] == SERVER_COMMAND_TELL || this->args[0] == SERVER_COMMAND_YELL || this->args[0] == SERVER_COMMAND_NAME) {
            return COMMAND_TYPE_SERVER;
        }
        return COMMAND_TYPE_ELSE;
    }

    server_command_type getServerCommandType() {
        if (this->args[0] == SERVER_COMMAND_WHO) {
            return SERVER_COMMAND_TYPE_WHO;
        } else if (this->args[0] == SERVER_COMMAND_TELL) {
            return SERVER_COMMAND_TYPE_TELL;
        } else if (this->args[0] == SERVER_COMMAND_YELL) {
            return SERVER_COMMAND_TYPE_YELL;
        } else if (this->args[0] == SERVER_COMMAND_NAME) {
            return SERVER_COMMAND_TYPE_NAME;
        }
        return SERVER_COMMAND_TYPE_ELSE;
    }

    void print() {
        for (auto arg : this->args) {
            cout << arg << " ";
        }
        cout << "\n";
        cout << "pipe: " << this->pipe << "\n";
        cout << "pipeNum: " << this->pipeNum << "\n";
        cout << "fileRedirect: " << this->fileRedirect << "\n";
        cout << "outputFile: " << this->outputFile << "\n";
        cout << "sendPipe: " << this->sendPipe << "\n";
        cout << "recvPipe: " << this->recvPipe << "\n";
        cout << "sendSqn: " << this->sendSqn << "\n";
        cout << "recvSqn: " << this->recvSqn << "\n";
        cout << "--------------------------------\n";
    }
};

struct commands {
    string originalInput;
    vector<command> cmds;

    commands(): originalInput(""), cmds() {}
    ~commands() {
        cmds.clear();
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

    void parseCommands(string input) {
        vector<string> parts = splitBySpace(input);
        if (parts.empty()) {
            return;
        }
        for (size_t i = 0; i < parts.size(); i++) {
            this->originalInput += parts[i];
            if (i != parts.size() - 1) {
                this->originalInput += " ";
            }
        }

        command cmd;
        for (size_t i = 0; i < parts.size(); i++) {
            cmd.originalInput += parts[i];
            if (i != parts.size() - 1) {
                cmd.originalInput += " ";
            }
        }
        if (parts[0] == SERVER_COMMAND_WHO || parts[0] == SERVER_COMMAND_TELL || parts[0] == SERVER_COMMAND_YELL || parts[0] == SERVER_COMMAND_NAME) {
            for (auto it = parts.begin(); it != parts.end(); it++) {
                string part = *it;
                cmd.addArg(part);
            }
        } else {
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
                } else if (part.size() > 1 && part[0] == '>' && isdigit(part[1])) {
                    cmd.setSendPipe(stoi(part.substr(1)));
                    continue;
                } else if (part.size() > 1 && part[0] == '<' && isdigit(part[1])) {
                    cmd.setRecvPipe(stoi(part.substr(1)));
                    continue;
                } else {
                    cmd.addArg(part);
                    continue;
                }
                this->cmds.push_back(cmd);
                cmd = command();
            }
        }

        if (!cmd.args.empty()) {
            this->cmds.push_back(cmd);
        }
    }

    bool isEmpty() {
        return this->cmds.empty() || this->cmds[0].args.empty();
    }

    command getFirstCommand() {
        return this->cmds[0];
    }

    string getOriginalInput() {
        return this->originalInput;
    }

    void print() {
        for (auto cmd : this->cmds) {
            cmd.print();
        }
    }
};

struct numberedPipe {
    int readFd;
    int writeFd;
    int count;

    bool isUsed;

    numberedPipe(int readFd, int writeFd, int count) : readFd(readFd), writeFd(writeFd), count(count), isUsed(false) {}
    ~numberedPipe() {}

    void closePipe() {
        if (this->readFd >= 0) {
            close(this->readFd);
        }
        if (this->writeFd >= 0) {
            close(this->writeFd);
        }
    }

    void print() {
        cout << "readFd: " << this->readFd << "\n";
        cout << "writeFd: " << this->writeFd << "\n";
        cout << "count: " << this->count << "\n";
        cout << "--------------------------------\n";
    }
};

struct numberedPipes {
    vector<numberedPipe> nbpps;

    numberedPipes() {}
    ~numberedPipes() {
        for (auto& pipe : nbpps) {
            pipe.closePipe();
        }
        nbpps.clear();
    }

    void addPipe(numberedPipe pipe) {
        nbpps.push_back(pipe);
    }

    void increaseCount() {
        for (auto& pipe : nbpps) {
            pipe.count++;
        }
    }

    void decreaseCount() {
        for (auto& pipe : nbpps) {
            pipe.count--;
        }
    }

    void resetCountOneToZero() {
        for (auto& pipe : nbpps) {
            if (pipe.count == 1) {
                pipe.count = 0;
            }
        }
    }

    void removeUsedPipes() {
        nbpps.erase(
            remove_if(nbpps.begin(), nbpps.end(), 
                [](const numberedPipe& p) { 
                    if (p.isUsed) {
                        close(p.readFd);
                        close(p.writeFd);
                    }
                return p.isUsed;
            }),
            nbpps.end()
        );
    }

    int getPipeWithCountIsZero() {
        for (int i = 0; i < nbpps.size(); i++) {
            if (nbpps[i].count == 0) {
                nbpps[i].isUsed = true;
                return i;
            }
        }
        return NOT_FOUND;
    }

    int getOutputPipeWithSameCount(int count) {
        for (auto pipe : nbpps) {
            if (pipe.count == count) {
                return pipe.writeFd;
            }
        }
        return NOT_FOUND;
    }

    numberedPipe getPipeWithIndex(int index) {
        return nbpps[index];
    }

    void print() {
        cout << "length: " << nbpps.size() << "\n";
        for (auto pipe : nbpps) {
            pipe.print();
        }
    }
};

struct tcpServerSocket {
    socketFd serverSocket;
    sockaddr_in serverAddress;

    tcpServerSocket() : serverSocket(-1) {}
    ~tcpServerSocket() {
        if (serverSocket >= 0) {
            close(serverSocket);
        }
    }

    error newTcpServerSocket(int port) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            return ERROR_SOCKET_CREATION_FAILED;
        }
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            return ERROR_SOCKET_OPTION_SETTING_FAILED;
        }
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);
        if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
            close(serverSocket);
            serverSocket = -1;
            return ERROR_SOCKET_BINDING_FAILED;
        }
        return nil;
    }

    error listenTcpServerSocket() {
        if (listen(serverSocket, MAX_CLIENTS_USERS) < 0) {
            return ERROR_SOCKET_LISTENING_FAILED;
        }
        return nil;
    }

    socketFd acceptTcpServerSocket(sockaddr_in &clientAddress) {
        socketFd clientSocket;
        socklen_t clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            return CLIENT_ERROR;
        }
        return clientSocket;
    }

    socketFd getServerSocket() {
        return serverSocket;
    }

    void closeSocket() {
        if (serverSocket >= 0) {
            close(serverSocket);
            serverSocket = -1;
        }
    }

    void print() {
        cout << "serverSocket: " << this->serverSocket << "\n";
        cout << "serverAddress: " << this->serverAddress.sin_addr.s_addr << "\n";
        cout << "serverAddress: " << this->serverAddress.sin_port << "\n";
        cout << "--------------------------------\n";
    }
};

struct user {
    int sequenceNumber;
    socketFd clientSocket;
    string name;
    string ip;
    string port;
    map<string, string> env;
    numberedPipes nbpps;

    user() : sequenceNumber(USER_DEFAULT_SQN), clientSocket(CLIENT_DISCONNECTED), name(USER_DEFAULT_NAME), ip(USER_DEFAULT_IP), port(USER_DEFAULT_PORT) {
        env[USER_DEFAULT_ENV_KEY] = USER_DEFAULT_ENV_VALUE;
    }
    ~user() {
        if (this->clientSocket >= 0) {
            close(this->clientSocket);
        }
        env.clear();
        nbpps.~numberedPipes();
    }

    void userInit() {
        this->sequenceNumber = USER_DEFAULT_SQN;
        this->clientSocket = CLIENT_DISCONNECTED;
        this->name = USER_DEFAULT_NAME;
        this->ip = USER_DEFAULT_IP;
        this->port = USER_DEFAULT_PORT;
        this->env = map<string, string>();
        this->env[USER_DEFAULT_ENV_KEY] = USER_DEFAULT_ENV_VALUE;
        this->nbpps = numberedPipes();
    }

    void updateUserEnv(string key, string value) {
        this->env[key] = value;
    }

    void setUserEnv() {
        for (auto [key, value] : this->env) {
            setenv(key.c_str(), value.c_str(), 1);
        }
    }

    void closeClientSocket() {
        if (this->clientSocket >= 0) {
            close(this->clientSocket);
        }
    }

    void print() {
        cout << "sequenceNumber: " << this->sequenceNumber << "\n";
        cout << "clientSocket: " << this->clientSocket << "\n";
        cout << "name: " << this->name << "\n";
        cout << "ip: " << this->ip << "\n";
        cout << "port: " << this->port << "\n";
        for (auto [key, value] : this->env) {
            cout << "key: " << key << " value: " << value << "\n";
        }
        cout << "--------------------------------\n";
    }
};

struct usersManagement {
    user users[MAX_CLIENTS_USERS];

    usersManagement() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            users[i] = user();
        }
    }
    ~usersManagement() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (users[i].clientSocket >= 0) {
                close(users[i].clientSocket);
            }
        }
    }

    int sequenceNumberGenerator() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (users[i].sequenceNumber == -1) {
                return i + 1;
            }
        }
        return NOT_FOUND;
    }

    int getUserIndexBySqn(int sqn) {
        return sqn - 1;
    }

    int getUserIndexByClientSocket(socketFd clientSocket) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (users[i].clientSocket == clientSocket) {
                return i;
            }
        }
        return NOT_FOUND;
    }

    int getUserSqnByIndex(int index) {
        return index + 1;
    }

    int getUserSqnByClientSocket(socketFd clientSocket) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (users[i].clientSocket == clientSocket) {
                return users[i].sequenceNumber;
            }
        }
        return NOT_FOUND;
    }

    bool isUserLogin(int sqn) {
        if (sqn < 1 || sqn > MAX_CLIENTS_USERS) {
            return false;
        }
        return users[getUserIndexBySqn(sqn)].sequenceNumber != USER_DEFAULT_SQN;
    }

    error updateUserEnv(socketFd clientSocket, string key, string value) {
        int index = getUserIndexByClientSocket(clientSocket);
        if (index == NOT_FOUND) {
            return ERROR_USER_NOT_FOUND;
        }
        users[index].updateUserEnv(key, value);
        return nil;
    }

    error setUserEnv(socketFd clientSocket) {
        clearenv();
        int index = getUserIndexByClientSocket(clientSocket);
        if (index == NOT_FOUND) {
            return ERROR_USER_NOT_FOUND;
        }
        users[index].setUserEnv();
        return nil;
    }

    error updateUserName(socketFd clientSocket, string name) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (users[i].name == name) {
                return ERROR_SERVER_MESSAGE_NAME_UPDATE_FAILED;
            }
        }

        int index = getUserIndexByClientSocket(clientSocket);
        if (index == NOT_FOUND) {
            return ERROR_USER_NOT_FOUND;
        }
        users[index].name = name;

        char buffer[BUFFER_SIZE];
        if (error err = broadcastMessage(serverMsgf(buffer, SERVER_MESSAGE_NAME_UPDATE_SUCCESS, users[index].ip.c_str(), users[index].port.c_str(), users[index].name.c_str())); err != nil) {
            return err;
        }
        return nil;
    }

    error userLogin(socketFd clientSocket, sockaddr_in &clientAddress) {
        int sqn = sequenceNumberGenerator();
        if (sqn == NOT_FOUND) {
            return ERROR_USER_LOGIN_FAILED_WITH_RUN_OUT_OF_SQN;
        }

        users[getUserIndexBySqn(sqn)].sequenceNumber = sqn;
        users[getUserIndexBySqn(sqn)].clientSocket = clientSocket;
        users[getUserIndexBySqn(sqn)].ip = inet_ntoa(clientAddress.sin_addr);
        users[getUserIndexBySqn(sqn)].port = to_string(ntohs(clientAddress.sin_port));

        if (error err = sendWelcomeMessage(clientSocket); err != nil) {
            return err;
        }
        
        char buffer[BUFFER_SIZE];
        if (error err = broadcastMessage(serverMsgf(buffer, SERVER_MESSAGE_LOGIN, users[getUserIndexBySqn(sqn)].name.c_str(), users[getUserIndexBySqn(sqn)].ip.c_str(), users[getUserIndexBySqn(sqn)].port.c_str())); err != nil) {
            return err;
        }

        if (error err = sendPromptMessage(clientSocket); err != nil) {
            return err;
        }

        cout << "User " << users[getUserIndexBySqn(sqn)].sequenceNumber << " login: " << users[getUserIndexBySqn(sqn)].name << " from " << users[getUserIndexBySqn(sqn)].ip << ":" << users[getUserIndexBySqn(sqn)].port << "\n";
        return nil;
    }

    error userLogout(socketFd clientSocket) {
        int index = getUserIndexByClientSocket(clientSocket);
        if (index == NOT_FOUND) {
            return ERROR_USER_LOGOUT_FAILED_WITH_USER_NOT_FOUND;
        }

        int sqn = users[index].sequenceNumber;
        string name = users[index].name;
        string ip = users[index].ip;
        string port = users[index].port;

        users[index].closeClientSocket();
        users[index].userInit();

        char buffer[BUFFER_SIZE];
        if (error err = broadcastMessage(serverMsgf(buffer, SERVER_MESSAGE_LOGOUT, name.c_str())); err != nil) {
            return err;
        }

        cout << "User " << sqn << " logout: " << name << " from " << ip << ":" << port << "\n";
        return nil;
    }

    error broadcastMessage(systemMsg message) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (users[i].clientSocket >= 0) {
                if (write(users[i].clientSocket, message, strlen(message)) < 0) {
                    return ERROR_SERVER_MESSAGE_BROADCAST_FAILED;
                }
            }
        }
        return nil;
    }

    error sendPromptMessage(socketFd clientSocket) {
        if (write(clientSocket, SERVER_MESSAGE_PROMPT, strlen(SERVER_MESSAGE_PROMPT)) < 0) {
            return ERROR_SERVER_MESSAGE_SEND_PROMPT_MESSAGE_FAILED;
        }
        return nil;
    }

    error sendWelcomeMessage(socketFd clientSocket) {
        if (write(clientSocket, SERVER_MESSAGE_WELCOME, strlen(SERVER_MESSAGE_WELCOME)) < 0) {
            return ERROR_SERVER_MESSAGE_SEND_WELCOME_MESSAGE_FAILED;
        }
        return nil;
    }

    error errorf(systemMsg buffer, const char* format, ...) {
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, BUFFER_SIZE, format, args);
        va_end(args);
        return buffer;
    }

    systemMsg serverMsgf(systemMsg buffer, const char* format, ...) {
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, BUFFER_SIZE, format, args);
        va_end(args);
        return buffer;
    }

    void print() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            users[i].print();
        }
    }
};

struct userPipe {
    int readFd;
    int writeFd;

    int sendSqn;
    int recvSqn;

    string originalInput;
    bool isUsed;

    userPipe(int readFd, int writeFd, int sendSqn, int recvSqn, string originalInput) : readFd(readFd), writeFd(writeFd), sendSqn(sendSqn), recvSqn(recvSqn), originalInput(originalInput), isUsed(false) {}
    ~userPipe() {}

    string getOriginalInput() {
        return this->originalInput;
    }

    void closePipe() {
        if (this->readFd >= 0) {
            close(this->readFd);
        }
        if (this->writeFd >= 0) {
            close(this->writeFd);
        }
    }

    void print() {
        cout << "readFd: " << this->readFd << "\n";
        cout << "writeFd: " << this->writeFd << "\n";
        cout << "sendSqn: " << this->sendSqn << "\n";
        cout << "recvSqn: " << this->recvSqn << "\n";
        cout << "--------------------------------\n";
    }
};

struct userPipes {
    vector<userPipe> uspps;

    userPipes() : uspps() {}
    ~userPipes() {
        for (auto& userPipe : uspps) {
            userPipe.closePipe();
        }
        uspps.clear();
    }

    void addUserPipe(userPipe userPipe) {
        uspps.push_back(userPipe);
    }

    void removeUserPipe(int sendSqn, int recvSqn) {
        uspps.erase(remove_if(uspps.begin(), uspps.end(), [sendSqn, recvSqn](userPipe userPipe) {
            return userPipe.sendSqn == sendSqn && userPipe.recvSqn == recvSqn;
        }), uspps.end());
    }

    void removeUserAccordingPipes(int sqn) {
        vector<pair<int, int>> pipesToRemove;
        
        for (auto userPipe : uspps) {
            if (userPipe.sendSqn == sqn || userPipe.recvSqn == sqn) {
                pipesToRemove.push_back({userPipe.sendSqn, userPipe.recvSqn});
            }
        }
        
        for (auto& pipe : pipesToRemove) {
            removeUserPipe(pipe.first, pipe.second);
        }
    }

    void removeUsedUserPipes() {
        vector<pair<int, int>> pipesToRemove;

        for (auto userPipe : uspps) {
            if (userPipe.isUsed) {
                pipesToRemove.push_back({userPipe.sendSqn, userPipe.recvSqn});
            }
        }

        for (auto pipe : pipesToRemove) {
            removeUserPipe(pipe.first, pipe.second);
        }
    }

    userPipe* getUserPipe(int sendSqn, int recvSqn) {
        for (auto& userPipe : uspps) {
            if (userPipe.sendSqn == sendSqn && userPipe.recvSqn == recvSqn) {
                return &userPipe;
            }
        }
        return nil;
    }

    userPipe* getUserPipeAndSetUsed(int sendSqn, int recvSqn) {
        for (auto& userPipe : uspps) {
            if (userPipe.sendSqn == sendSqn && userPipe.recvSqn == recvSqn) {
                userPipe.isUsed = true;
                return &userPipe;
            }
        }
        return nil;
    }

    void print() {
        for (auto userPipe : uspps) {
            userPipe.print();
        }
    }
};

#endif