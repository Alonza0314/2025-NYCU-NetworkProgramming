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

    bool isBusy;
    bool isUsed;

    numberedPipe() : readFd(NOT_FOUND), writeFd(NOT_FOUND), count(0), isBusy(false), isUsed(false) {}
    numberedPipe(int readFd, int writeFd, int count) : readFd(readFd), writeFd(writeFd), count(count), isBusy(false), isUsed(false) {}
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
    numberedPipe nbpps[BUFFER_SIZE];

    numberedPipes() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            nbpps[i] = numberedPipe();
        }
    }
    ~numberedPipes() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            nbpps[i].closePipe();
        }
    }

    int generatePipeIndex() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (!nbpps[i].isBusy) {
                return i;
            }
        }
        return NOT_FOUND;
    }

    void addPipe(numberedPipe pipe) {
        int index = generatePipeIndex();
        if (index == NOT_FOUND) {
            return;
        }
        nbpps[index].isBusy = true;
        nbpps[index].readFd = pipe.readFd;
        nbpps[index].writeFd = pipe.writeFd;
        nbpps[index].count = pipe.count;
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
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (nbpps[i].count == 1) {
                nbpps[i].count = 0;
            }
        }
    }

    void removeUsedPipes() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (nbpps[i].isUsed) {
                close(nbpps[i].readFd);
                close(nbpps[i].writeFd);
                nbpps[i].readFd = NOT_FOUND;
                nbpps[i].writeFd = NOT_FOUND;
                nbpps[i].isBusy = false;
                nbpps[i].isUsed = false;
            }
        }
    }

    int getPipeWithCountIsZero() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (nbpps[i].count == 0) {
                nbpps[i].isUsed = true;
                return i;
            }
        }
        return NOT_FOUND;
    }

    int getOutputPipeWithSameCount(int count) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (nbpps[i].count == count) {
                return nbpps[i].writeFd;
            }
        }
        return NOT_FOUND;
    }

    numberedPipe getPipeWithIndex(int index) {
        return nbpps[index];
    }

    void print() {
        cout << "length: " << BUFFER_SIZE << "\n";
        for (int i = 0; i < BUFFER_SIZE; i++) {
            nbpps[i].print();
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
    int pid;
    int sequenceNumber;
    socketFd clientSocket;
    char name[BUFFER_SIZE];
    char ip[BUFFER_SIZE];
    char port[BUFFER_SIZE];
    numberedPipes nbpps;

    user() : sequenceNumber(USER_DEFAULT_SQN), clientSocket(CLIENT_DISCONNECTED) {
        strncpy(name, USER_DEFAULT_NAME.c_str(), BUFFER_SIZE - 1);
        name[USER_DEFAULT_NAME.length()] = '\0';
        strncpy(ip, USER_DEFAULT_IP.c_str(), BUFFER_SIZE - 1);
        ip[USER_DEFAULT_IP.length()] = '\0';
        strncpy(port, USER_DEFAULT_PORT.c_str(), BUFFER_SIZE - 1);
        port[USER_DEFAULT_PORT.length()] = '\0';
        nbpps = numberedPipes();
    }
    ~user() {
        if (this->clientSocket >= 0) {
            close(this->clientSocket);
        }
        nbpps.~numberedPipes();
    }

    void userInit() {
        this->pid = NOT_FOUND;
        this->sequenceNumber = USER_DEFAULT_SQN;
        this->clientSocket = CLIENT_DISCONNECTED;
        strncpy(this->name, USER_DEFAULT_NAME.c_str(), BUFFER_SIZE - 1);
        this->name[USER_DEFAULT_NAME.length()] = '\0';
        strncpy(this->ip, USER_DEFAULT_IP.c_str(), BUFFER_SIZE - 1);
        this->ip[USER_DEFAULT_IP.length()] = '\0';
        strncpy(this->port, USER_DEFAULT_PORT.c_str(), BUFFER_SIZE - 1);
        this->port[USER_DEFAULT_PORT.length()] = '\0';
        this->nbpps = numberedPipes();
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
        cout << "--------------------------------\n";
    }
};

struct userFifo {
    int writeSqn;
    int readSqn;
    int readFd;
    char fifoName[BUFFER_SIZE];
    bool busy;
    bool used;

    userFifo() : writeSqn(NOT_FOUND), readSqn(NOT_FOUND), readFd(NOT_FOUND), busy(false), used(false) {
        fifoName[0] = '\0';
    }
    ~userFifo() {
        if (this->fifoName[0] != '\0') {
            unlink(this->fifoName);
        }
        if (this->readFd > 0) {
            close(this->readFd);
        }
    }

    void init() {
        this->writeSqn = NOT_FOUND;
        this->readSqn = NOT_FOUND;
        if (this->readFd > 0) {
            close(this->readFd);
        }
        this->readFd = NOT_FOUND;
        if (this->fifoName[0] != '\0') {
            unlink(this->fifoName);
        }
        this->fifoName[0] = '\0';
        this->busy = false;
        this->used = false;
    }

    int getReadFd() {
        return this->readFd;
    }

    bool userFifoExists(int writeSqn, int readSqn) {
        return this->writeSqn == writeSqn && this->readSqn == readSqn;
    }

    bool isBusy() {
        return this->busy;
    }

    bool isUsed() {
        return this->used;
    }

    bool writeFdNotOpenButBusy() {
        return this->writeSqn != NOT_FOUND && this->readSqn != NOT_FOUND && this->readFd == NOT_FOUND && this->busy;
    }

    void openReadFd() {
        this->readFd = open(this->fifoName, O_RDONLY);
        if (this->readFd < 0) {
            this->readFd = NOT_FOUND;
            cerr << ERROR_OPEN_USER_PIPE_READ_FAILED;
        }
    }

    void setUsed() {
        this->used = true;
    }

    string makeFifoName(int writeSqn, int readSqn) {
        return USER_PIPE_DIR + "/" + to_string(writeSqn) + "_" + to_string(readSqn);
    }

    void print() {
        if (!this->busy) {
            return;
        }
        cout << "writeSqn: " << this->writeSqn << "\n";
        cout << "readSqn: " << this->readSqn << "\n";
        cout << "readFd: " << this->readFd << "\n";
        cout << "fifoName: " << this->fifoName << "\n";
        cout << "busy: " << this->busy << "\n";
        cout << "used: " << this->used << "\n";
        cout << "--------------------------------\n";
    }
};

struct usersManagement {
    char broadcastBuffer[BUFFER_SIZE];
    userFifo userFifos[BUFFER_SIZE];
    user users[MAX_CLIENTS_USERS];

    usersManagement() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            users[i] = user();
        }
        strncpy(broadcastBuffer, "", BUFFER_SIZE - 1);
        broadcastBuffer[0] = '\0';
        for (int i = 0; i < BUFFER_SIZE; i++) {
            userFifos[i] = userFifo();
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

    int getUserPidBySqn(int sqn) {
        return users[getUserIndexBySqn(sqn)].pid;
    }

    void setUserPid(int sqn, int pid) {
        users[getUserIndexBySqn(sqn)].pid = pid;
    }

    char* getBroadcastBuffer() {
        return this->broadcastBuffer;
    }

    bool isUserLogin(int sqn) {
        if (sqn < 1 || sqn > MAX_CLIENTS_USERS) {
            return false;
        }
        return users[getUserIndexBySqn(sqn)].sequenceNumber != USER_DEFAULT_SQN;
    }

    error updateUserName(int sqn, string name) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (strcmp(users[i].name, name.c_str()) == 0) {
                return ERROR_SERVER_MESSAGE_NAME_UPDATE_FAILED;
            }
        }

        int index = getUserIndexBySqn(sqn);
        if (index == NOT_FOUND) {
            return ERROR_USER_NOT_FOUND;
        }
        strcpy(users[index].name, name.c_str());

        char buffer[BUFFER_SIZE];
        if (error err = broadcastMessage(serverMsgf(buffer, SERVER_MESSAGE_NAME_UPDATE_SUCCESS, users[index].ip, users[index].port, users[index].name)); err != nil) {
            return err;
        }
        return nil;
    }

    error userLogin(socketFd clientSocket, sockaddr_in &clientAddress, int *sqn) {
        *sqn = sequenceNumberGenerator();
        if (*sqn == NOT_FOUND) {
            return ERROR_USER_LOGIN_FAILED_WITH_RUN_OUT_OF_SQN;
        }

        users[getUserIndexBySqn(*sqn)].sequenceNumber = *sqn;
        users[getUserIndexBySqn(*sqn)].clientSocket = clientSocket;
        strcpy(users[getUserIndexBySqn(*sqn)].ip, inet_ntoa(clientAddress.sin_addr));
        strcpy(users[getUserIndexBySqn(*sqn)].port, to_string(ntohs(clientAddress.sin_port)).c_str());

        cout << "User " << users[getUserIndexBySqn(*sqn)].sequenceNumber << " login: " << users[getUserIndexBySqn(*sqn)].name << " from " << users[getUserIndexBySqn(*sqn)].ip << ":" << users[getUserIndexBySqn(*sqn)].port << "\n";
        return nil;
    }

    error userLogout(int sqn) {
        int index = getUserIndexBySqn(sqn);
        if (index == NOT_FOUND) {
            return ERROR_USER_LOGOUT_FAILED_WITH_USER_NOT_FOUND;
        }

        string name = users[index].name;
        string ip = users[index].ip;
        string port = users[index].port;

        users[index].closeClientSocket();
        users[index].userInit();

        cout << "User " << sqn << " logout: " << name << " from " << ip << ":" << port << "\n";
        return nil;
    }

    error broadcastMessage(systemMsg message) {
        strncpy(broadcastBuffer, message, BUFFER_SIZE - 1);
        broadcastBuffer[strlen(message)] = '\0';
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (isUserLogin(getUserSqnByIndex(i))) {
                kill(getUserPidBySqn(getUserSqnByIndex(i)), SIGUSR1);
            }
        }

        // wait for all processes to receive the signal & broadcast message
        usleep(2000);
        strncpy(broadcastBuffer, "", BUFFER_SIZE - 1);
        broadcastBuffer[0] = '\0';
        return nil;
    }

    void sendPromptMessage() {
        cout << SERVER_MESSAGE_PROMPT;
    }

    error sendWelcomeMessage(socketFd clientSocket) {
        if (write(clientSocket, SERVER_MESSAGE_WELCOME, strlen(SERVER_MESSAGE_WELCOME)) < 0) {
            return ERROR_SERVER_MESSAGE_SEND_WELCOME_MESSAGE_FAILED;
        }
        return nil;
    }

    int getUserFifoIndex(int writeSqn, int readSqn) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (userFifos[i].isBusy() && userFifos[i].userFifoExists(writeSqn, readSqn)) {
                return i;
            }
        }
        return NOT_FOUND;
    }

    int generateUserFifoIndex() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (!userFifos[i].isBusy() && !userFifos[i].isUsed()) {
                return i;
            }
        }
        return NOT_FOUND;
    }

    void setUserFifoByIndex(int index, int writeSqn, int readSqn) {
        userFifos[index].writeSqn = writeSqn;
        userFifos[index].readSqn = readSqn;
        userFifos[index].busy = true;

        string fifoName = userFifos[index].makeFifoName(writeSqn, readSqn);
        strncpy(userFifos[index].fifoName, fifoName.c_str(), fifoName.length());
        userFifos[index].fifoName[fifoName.length()] = '\0';
    }

    void setUserFifoUsedByIndex(int index) {
        userFifos[index].setUsed();
    }

    void removeUsedUserFifo() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (userFifos[i].isUsed()) {
                userFifos[i].init();
            }
        }
    }

    void removeUserRelatedFifo(int sqn) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (userFifos[i].isBusy() && (userFifos[i].writeSqn == sqn || userFifos[i].readSqn == sqn)) {
                userFifos[i].init();
            }
        }
    }

    void signalTargetUserToOpenReadFd(int sqn) {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            if (isUserLogin(getUserSqnByIndex(i)) && users[i].sequenceNumber == sqn) {
                kill(getUserPidBySqn(getUserSqnByIndex(i)), SIGUSR2);
                break;
            }
        }

        usleep(2000);
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

    void printUsers() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            users[i].print();
        }
    }

    void printUserFifos() {
        for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
            userFifos[i].print();
        }
    }
};

struct sharedMemory {
    int shmId;

    sharedMemory() : shmId(NOT_FOUND) {}
    ~sharedMemory() {}

    error createShm() {
        this->shmId = shmget(SHM_KEY, sizeof(usersManagement), IPC_CREAT | 0664);
        if (this->shmId == NOT_FOUND) {
            return ERROR_SHM_CREATE_FAILED;
        }

        usersManagement* uManagement = nil;
        if (error err = attachShm(&uManagement); err != nil) {
            return err;
        }

        if (uManagement != nil) {
            for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
                uManagement->users[i].userInit();
            }
            for (int i = 0; i < BUFFER_SIZE; i++) {
                uManagement->userFifos[i].init();
            }
        } else {
            detachShm(&uManagement);
            return ERROR_SHM_INIT_FAILED;
        }
        
        if (error err = detachShm(&uManagement); err != nil) {
            return err;
        }
        return nil;
    }

    error attachShm(usersManagement** usersManagementPtr) {
        if (*usersManagementPtr != nil) {
            return nil;
        }
        void* ptr = shmat(this->shmId, NULL, 0);
        if (ptr == (void*)-1) {
            return ERROR_SHM_ATTACH_FAILED;
        }
        *usersManagementPtr = static_cast<usersManagement*>(ptr);
        return nil;
    }

    error detachShm(usersManagement** usersManagementPtr) {
        if (usersManagementPtr == nil || *usersManagementPtr == nil) {
            return nil;
        }

        if (shmdt(*usersManagementPtr) == -1) {
            return ERROR_SHM_DETACH_FAILED;
        }
        *usersManagementPtr = nil;
        return nil;
    }

    error deleteShm() {
        if (this->shmId != NOT_FOUND) {
            if (shmctl(this->shmId, IPC_RMID, NULL) == -1) {
                return ERROR_SHM_DELETE_FAILED;
            }
            this->shmId = NOT_FOUND;
        }
        return nil;
    }
};

#endif