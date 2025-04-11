#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "header.hpp"
#include "const.hpp"
#include "util.hpp"
#include "model.hpp"

error processInputPrintEnv(command cmd) {
    if (cmd.args.size() != 2) {
        return ERROR_INVALID_NUMBER_OF_ARGUMENTS;
    }

    string var = cmd.args[1];

    char* output = getenv(var.c_str());
    if (output != nil) {
        cout << output << "\n";
    }

    return nil;
}

error processInputSetEnv(command cmd, socketFd clientSocket, usersManagement *uManagement) {
    if (cmd.args.size() != 3) {
        return ERROR_INVALID_NUMBER_OF_ARGUMENTS;
    }

    string var = cmd.args[1];
    string value = cmd.args[2];

    if (setenv(var.c_str(), value.c_str(), 1) < 0) {
        return ERROR_SETENV_FAILED;
    }

    if (error err = uManagement->updateUserEnv(clientSocket, var, value); err != nil) {
        return err;
    }

    return nil;
}

error processInputServerWho(command cmd, socketFd clientSocket, usersManagement *uManagement) {
    user* users = uManagement->users;
    int index = uManagement->getUserIndexByClientSocket(clientSocket);
    if (index == NOT_FOUND) {
        return ERROR_USER_NOT_FOUND;
    }
    if (write(clientSocket, SERVER_MESSAGE_WHO_TITLE, strlen(SERVER_MESSAGE_WHO_TITLE)) < 0) {
        return ERROR_SERVER_MESSAGE_SEND_WHO_TITLE_FAILED;
    }
    for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
        char buffer[BUFFER_SIZE];
        if (i == index) {
            uManagement->serverMsgf(buffer, SERVER_MESSAGE_WHO_USER_SELF, users[i].sequenceNumber, users[i].name.c_str(), users[i].ip.c_str(), users[i].port.c_str());
            if (write(clientSocket, buffer, strlen(buffer)) < 0) {
                return ERROR_SERVER_MESSAGE_SEND_WHO_USER_FAILED;
            }
        } else if (users[i].sequenceNumber != USER_DEFAULT_SQN) {
            uManagement->serverMsgf(buffer, SERVER_MESSAGE_WHO_USER, users[i].sequenceNumber, users[i].name.c_str(), users[i].ip.c_str(), users[i].port.c_str());
            if (write(clientSocket, buffer, strlen(buffer)) < 0) {
                return ERROR_SERVER_MESSAGE_SEND_WHO_USER_FAILED;
            }
        }
    }
    return nil;
}

error processInputServerTell(command cmd, socketFd clientSocket, usersManagement *uManagement) {
    if (cmd.args.size() < 2) {
        return ERROR_INVALID_NUMBER_OF_ARGUMENTS;
    }
    string sqn = cmd.args[1];
    if (!isNumber(sqn)) {
        return ERROR_ID_NOT_NUMBER;
    }

    if (!uManagement->isUserLogin(stoi(sqn))) {
        char buffer[BUFFER_SIZE];
        uManagement->serverMsgf(buffer, SERVER_MESSAGE_TELL_FAILED, sqn.c_str());
        if (write(clientSocket, buffer, strlen(buffer)) < 0) {
            return ERROR_SERVER_MESSAGE_SEND_TELL_FAILED;
        }
        return nil;
    }

    int index = uManagement->getUserIndexBySqn(stoi(sqn));
    string msg = "";
    for (int i = 2; i < cmd.args.size(); i++) {
        msg += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            msg += " ";
        }
    }

    char buffer[BUFFER_SIZE];
    uManagement->serverMsgf(buffer, SERVER_MESSAGE_TELL_SUCCESS, uManagement->users[uManagement->getUserIndexByClientSocket(clientSocket)].name.c_str(), msg.c_str());
    if (write(uManagement->users[index].clientSocket, buffer, strlen(buffer)) < 0) {
        return ERROR_SERVER_MESSAGE_SEND_TELL_FAILED;
    }
    return nil;
}

error processInputServerYell(command cmd, socketFd clientSocket, usersManagement *uManagement) {
    string msg = "";
    for (int i = 1; i < cmd.args.size(); i++) {
        msg += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            msg += " ";
        }
    }

    char buffer[BUFFER_SIZE];
    uManagement->serverMsgf(buffer, SERVER_MESSAGE_YELL, uManagement->users[uManagement->getUserIndexByClientSocket(clientSocket)].name.c_str(), msg.c_str());
    if (error err = uManagement->broadcastMessage(buffer); err != nil) {
        return err;
    }
    return nil;
}

error processInputServerName(command cmd, socketFd clientSocket, usersManagement *uManagement) {
    string name = "";
    for (int i = 1; i < cmd.args.size(); i++) {
        name += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            name += " ";
        }
    }

    if (error err = uManagement->updateUserName(clientSocket, name); err != nil) {
        if (err == ERROR_SERVER_MESSAGE_NAME_UPDATE_FAILED) {
            char buffer[BUFFER_SIZE];
            uManagement->serverMsgf(buffer, SERVER_MESSAGE_NAME_UPDATE_FAILED, name.c_str());
            if (write(clientSocket, buffer, strlen(buffer)) < 0) {
                return err;
            }
            return nil;
        }
        return err;
    }

    return nil;
}

error processInputServer(command cmd, socketFd clientSocket, usersManagement *uManagement) {
    switch (cmd.getServerCommandType()) {
    case SERVER_COMMAND_TYPE_WHO:
        return processInputServerWho(cmd, clientSocket, uManagement);
    case SERVER_COMMAND_TYPE_TELL:
        return processInputServerTell(cmd, clientSocket, uManagement);
    case SERVER_COMMAND_TYPE_YELL:
        return processInputServerYell(cmd, clientSocket, uManagement);
    case SERVER_COMMAND_TYPE_NAME:
        return processInputServerName(cmd, clientSocket, uManagement);
    }
    return nil;
}

error processInputMain(command cmd, int readFd, int writeFd, int meFd, string originalInput, numberedPipes *nbpps, userPipes *uspps, usersManagement *uManagement) {
    // process numbered pipe
    int numberedPipeFd[2] = {-1, -1};
    int outputToPipeFd = NOT_FOUND;
    if (!cmd.isFileRedirect() && cmd.isPipe()) {
        if (!cmd.isPipeNormal()) {
            outputToPipeFd = nbpps->getOutputPipeWithSameCount(cmd.pipeNum);
        } else {
            nbpps->increaseCount();
            nbpps->resetCountOneToZero();
        }
        if (outputToPipeFd == NOT_FOUND) {
            // if there is no pipe with the same number, create a new pipe
            if (pipe(numberedPipeFd) < 0) {
                return ERROR_PIPE_FAILED;
            }
            nbpps->addPipe(numberedPipe(numberedPipeFd[0], numberedPipeFd[1], cmd.pipeNum));
        } else {
            numberedPipeFd[1] = outputToPipeFd;
        }
    }

    // process user pipe
    int userPipeFd[2] = {-1, -1};
    int meIndex = uManagement->getUserIndexByClientSocket(meFd);
    int meSqn = uManagement->getUserSqnByIndex(meIndex);
    if (cmd.hasRecvPipe()) {
        error buffer = new char[BUFFER_SIZE];
        if (!uManagement->isUserLogin(cmd.recvSqn)) {
            cerr << uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_USER_NOT_EXIST, cmd.recvSqn);
            readFd = open("/dev/null", O_RDWR);
        } else {
            string recvOriginalInput = "";
            if (userPipe* uPipe = uspps->getUserPipeAndSetUsed(cmd.recvSqn, meSqn); uPipe != nil) {
                readFd = uPipe->readFd;
                writeFd = uPipe->writeFd;
                recvOriginalInput = uPipe->originalInput;
            } else {
                return uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_NOT_EXISTS_FAILED, cmd.recvSqn, meSqn);
            }
            uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_USER_PIPE_RECV_SUCCESS, uManagement->users[meIndex].name.c_str(), meSqn, uManagement->users[uManagement->getUserIndexBySqn(cmd.recvSqn)].name.c_str(), cmd.recvSqn, cmd.getOriginalInput().c_str()));
        }
    }
    if (cmd.hasSendPipe()) {
        error buffer = new char[BUFFER_SIZE];
        if (!uManagement->isUserLogin(cmd.sendSqn)) {
            cerr << uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_USER_NOT_EXIST, cmd.sendSqn);
            userPipeFd[1] = open("/dev/null", O_RDWR);
        } else {
            if (userPipe* uPipe = uspps->getUserPipe(meSqn, cmd.sendSqn); uPipe != nil) {
                return uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_ALREADY_EXISTS_FAILED, meSqn, cmd.sendSqn);
            }
            if (pipe(userPipeFd) < 0) {
                return ERROR_PIPE_FAILED;
            }
            uspps->addUserPipe(userPipe(userPipeFd[0], userPipeFd[1], meSqn, cmd.sendSqn, originalInput));
            uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_USER_PIPE_SEND_SUCCESS, uManagement->users[meIndex].name.c_str(), meSqn, originalInput.c_str(), uManagement->users[uManagement->getUserIndexBySqn(cmd.sendSqn)].name.c_str(), cmd.sendSqn));
        }
    }

    pid_t pid;
    // if fork() failed, wait for any one child process to finish
    while ((pid = fork()) == -1) {
        waitpid(-1, NULL, 0);
    }
    if (pid == 0) {
        // child process
        if (cmd.isFileRedirect()) {
            int fd = open(cmd.outputFile.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0777);
            if (fd < 0) {
                return ERROR_OPEN_FILE_FAILED;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else if (cmd.isPipe()) {
            dup2(numberedPipeFd[1], STDOUT_FILENO);
            if (cmd.isPipeNumberedErr()) {
                dup2(numberedPipeFd[1], STDERR_FILENO);
            }
            close(numberedPipeFd[0]);
            close(numberedPipeFd[1]);
        } else if (cmd.hasSendPipe()) {
            dup2(userPipeFd[1], STDOUT_FILENO);
            close(userPipeFd[0]);
            close(userPipeFd[1]);
        }

        if (readFd != NOT_FOUND) {
            dup2(readFd, STDIN_FILENO);
            close(readFd);
        }
        if (writeFd != NOT_FOUND) {
            close(writeFd);
        }

        char* args[cmd.args.size() + 1];
        for (size_t i = 0; i < cmd.args.size(); i++) {
            args[i] = strdup(cmd.args[i].c_str());
        }
        args[cmd.args.size()] = NULL;

        if (execvp(args[0], args) < 0) {
            cerr << "Unknown command: [" + cmd.args[0] + "].\n";
            exit(1);
        }
        exit(0);
    } else {
        // parent process
        if (readFd != NOT_FOUND) {
            close(readFd);
        }
        if (writeFd != NOT_FOUND) {
            close(writeFd);
        }

        if (!cmd.isPipe() && !cmd.hasSendPipe()) {
            if (waitpid(pid, nil, 0) < 0) {
                return ERROR_WAITPID_FAILED;
            }
        }
    }
    return nil;
}

void processInput(string input, numberedPipes *nbpps, socketFd clientSocket, usersManagement *uManagement, userPipes *uspps) {
    error err = nil;
    commands cmds;
    cmds.parseCommands(input);
    if (cmds.isEmpty()) {
        return;
    }

    switch (cmds.getFirstCommand().getCommandType()) {
    case COMMAND_TYPE_PRINT_ENV:
        if ((err = processInputPrintEnv(cmds.getFirstCommand())) && (err != nil)) {
            cerr << err;
        }
        break;
    case COMMAND_TYPE_SET_ENV:
        if ((err = processInputSetEnv(cmds.getFirstCommand(), clientSocket, uManagement)) && (err != nil)) {
            cerr << err;
        }
        break;
    case COMMAND_TYPE_SERVER:
        if ((err = processInputServer(cmds.getFirstCommand(), clientSocket, uManagement)) && (err != nil)) {
            cerr << err;
        }
        break;
    default:
        bool isPipe = false;
        for (auto cmd : cmds.cmds) {
            // decrement the pipe counts and release invalid pipes
            nbpps->decreaseCount();

            // remove user pipes that has been used
            uspps->removeUsedUserPipes();

            // check if there exist input from pipe, if so, process it
            int pipeIndex = nbpps->getPipeWithCountIsZero();
            if (pipeIndex != NOT_FOUND) {
                isPipe = true;
                if ((err = processInputMain(cmd, nbpps->getPipeWithIndex(pipeIndex).readFd, nbpps->getPipeWithIndex(pipeIndex).writeFd, clientSocket, cmds.getOriginalInput(), nbpps, uspps, uManagement)) && (err != nil)) {
                    cerr << err;
                    break;
                }
            }

            // if there is no input from pipe, process the command as normal
            if (!isPipe) {
                if ((err = processInputMain(cmd, NOT_FOUND, NOT_FOUND, clientSocket, cmds.getOriginalInput(), nbpps, uspps, uManagement)) && (err != nil)) {
                    cerr << err;
                    break;
                }
            }
            nbpps->removeUsedPipes();
        }
    }
}

error processClientConnection(socketFd clientSocket, usersManagement *uManagement, userPipes *uspps) {
    int std[3] = {dup(STDIN_FILENO), dup(STDOUT_FILENO), dup(STDERR_FILENO)};

    dup2(clientSocket, STDOUT_FILENO);
    dup2(clientSocket, STDERR_FILENO);
    dup2(clientSocket, STDIN_FILENO);

    error err = nil;
    string input;
    if (getAndCheckIsExitAndEof(input)) {
        err = ERROR_USER_DISCONNECTED;
        goto RETURN;
    }
    if ((err = uManagement->setUserEnv(clientSocket)) && (err != nil)) {
        goto RETURN;
    }
    processInput(input, &uManagement->users[uManagement->getUserIndexByClientSocket(clientSocket)].nbpps, clientSocket, uManagement, uspps);
    if ((err = uManagement->sendPromptMessage(clientSocket)) && (err != nil)) {
        goto RETURN;
    }

RETURN:
    dup2(std[0], STDIN_FILENO);
    dup2(std[1], STDOUT_FILENO);
    dup2(std[2], STDERR_FILENO);

    close(std[0]);
    close(std[1]);
    close(std[2]);

    return err;
}

#endif
