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

error processInputSetEnv(command cmd, usersManagement *uManagement) {
    if (cmd.args.size() != 3) {
        return ERROR_INVALID_NUMBER_OF_ARGUMENTS;
    }

    string var = cmd.args[1];
    string value = cmd.args[2];

    if (setenv(var.c_str(), value.c_str(), 1) < 0) {
        return ERROR_SETENV_FAILED;
    }

    return nil;
}

error processInputServerWho(command cmd, int sqn, usersManagement *uManagement) {
    user* users = uManagement->users;
    int index = uManagement->getUserIndexBySqn(sqn);
    if (index == NOT_FOUND) {
        return ERROR_USER_NOT_FOUND;
    }
    cout << SERVER_MESSAGE_WHO_TITLE;
    for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
        char buffer[BUFFER_SIZE];
        if (i == index) {
            uManagement->serverMsgf(buffer, SERVER_MESSAGE_WHO_USER_SELF, users[i].sequenceNumber, users[i].name, users[i].ip, users[i].port);
            cout << buffer;
        } else if (users[i].sequenceNumber != USER_DEFAULT_SQN) {
            uManagement->serverMsgf(buffer, SERVER_MESSAGE_WHO_USER, users[i].sequenceNumber, users[i].name, users[i].ip, users[i].port);
            cout << buffer;
        }
    }
    return nil;
}

error processInputServerTell(command cmd, int sqn, usersManagement *uManagement) {
    if (cmd.args.size() < 2) {
        return ERROR_INVALID_NUMBER_OF_ARGUMENTS;
    }
    string tgtSqn = cmd.args[1];
    if (!isNumber(tgtSqn)) {
        return ERROR_ID_NOT_NUMBER;
    }

    if (!uManagement->isUserLogin(stoi(tgtSqn))) {
        char buffer[BUFFER_SIZE];
        uManagement->serverMsgf(buffer, SERVER_MESSAGE_TELL_FAILED, tgtSqn.c_str());
        cerr << buffer;
        return nil;
    }

    int pid = uManagement->getUserPidBySqn(stoi(tgtSqn));
    string msg = "";
    for (int i = 2; i < cmd.args.size(); i++) {
        msg += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            msg += " ";
        }
    }

    char buffer[BUFFER_SIZE];
    strncpy(uManagement->broadcastBuffer, uManagement->serverMsgf(buffer, SERVER_MESSAGE_TELL_SUCCESS, uManagement->users[uManagement->getUserIndexBySqn(sqn)].name, msg.c_str()), BUFFER_SIZE);
    kill(pid, SIGUSR1);

    // wait for the process to receive the signal and the target message
    usleep(2000);
    strncpy(uManagement->broadcastBuffer, "", BUFFER_SIZE - 1);
    uManagement->broadcastBuffer[0] = '\0';
    return nil;
}

error processInputServerYell(command cmd, int sqn, usersManagement *uManagement) {
    string msg = "";
    for (int i = 1; i < cmd.args.size(); i++) {
        msg += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            msg += " ";
        }
    }

    char buffer[BUFFER_SIZE];
    if (error err = uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_YELL, uManagement->users[uManagement->getUserIndexBySqn(sqn)].name, msg.c_str())); err != nil) {
        return err;
    }
    return nil;
}

error processInputServerName(command cmd, int sqn, usersManagement *uManagement) {
    string name = "";
    for (int i = 1; i < cmd.args.size(); i++) {
        name += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            name += " ";
        }
    }

    if (error err = uManagement->updateUserName(sqn, name); err != nil) {
        if (err == ERROR_SERVER_MESSAGE_NAME_UPDATE_FAILED) {
            char buffer[BUFFER_SIZE];
            uManagement->serverMsgf(buffer, SERVER_MESSAGE_NAME_UPDATE_FAILED, name.c_str());
            cerr << buffer;
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

error processInputMain(command cmd, int readFd, int writeFd, int meSqn, string originalInput, numberedPipes *nbpps, usersManagement *uManagement) {
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
    int meIndex = uManagement->getUserIndexBySqn(meSqn);
    if (cmd.hasRecvPipe()) {
        error buffer = new char[BUFFER_SIZE];
        if (!uManagement->isUserLogin(cmd.recvSqn)) {
            cerr << uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_USER_NOT_EXIST, cmd.recvSqn);
            readFd = open("/dev/null", O_RDWR);
        } else {
            if (int uFifoIndex = uManagement->getUserFifoIndex(cmd.recvSqn, meSqn); uFifoIndex == NOT_FOUND) {
                return uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_NOT_EXISTS_FAILED, cmd.recvSqn, meSqn);
            } else {
                uManagement->setUserFifoUsedByIndex(uFifoIndex);
                readFd = uManagement->userFifos[uFifoIndex].readFd;
                uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_USER_PIPE_RECV_SUCCESS, uManagement->users[meIndex].name, meSqn, uManagement->users[uManagement->getUserIndexBySqn(cmd.recvSqn)].name, cmd.recvSqn, cmd.getOriginalInput().c_str()));
            }
        }
    }
    if (cmd.hasSendPipe()) {
        error buffer = new char[BUFFER_SIZE];
        if (!uManagement->isUserLogin(cmd.sendSqn)) {
            cerr << uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_USER_NOT_EXIST, cmd.sendSqn);
            userPipeFd[1] = open("/dev/null", O_RDWR);
        } else {
            if (int uFifoIndex = uManagement->getUserFifoIndex(meSqn, cmd.sendSqn); uFifoIndex != NOT_FOUND) {
                return uManagement->errorf(buffer, ERROR_SERVER_MESSAGE_USER_PIPE_ALREADY_EXISTS_FAILED, meSqn, cmd.sendSqn);
            } else {
                uFifoIndex = uManagement->generateUserFifoIndex();

                uManagement->setUserFifoByIndex(uFifoIndex, meSqn, cmd.sendSqn);
                mkfifo(uManagement->userFifos[uFifoIndex].fifoName, 0666);
                uManagement->signalTargetUserToOpenReadFd(cmd.sendSqn);
                if ((userPipeFd[1] = open(uManagement->userFifos[uFifoIndex].fifoName, O_WRONLY)) < 0) {
                    return ERROR_OPEN_USER_PIPE_WRITE_FAILED;
                }
    
                uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_USER_PIPE_SEND_SUCCESS, uManagement->users[meIndex].name, meSqn, originalInput.c_str(), uManagement->users[uManagement->getUserIndexBySqn(cmd.sendSqn)].name, cmd.sendSqn));
            }
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

        if (userPipeFd[1] != NOT_FOUND) {
            close(userPipeFd[1]);
        }

        if (!cmd.isPipe() && !cmd.hasSendPipe()) {
            if (waitpid(pid, nil, 0) < 0) {
                return ERROR_WAITPID_FAILED;
            }
        }
    }
    return nil;
}

void processInput(string input, numberedPipes *nbpps, int sqn, usersManagement *uManagement) {
    error err = nil;
    commands cmds;
    cmds.parseCommands(input);
    if (cmds.isEmpty()) {
        return;
    }

    switch (cmds.getFirstCommand().getCommandType()) {
    case COMMAND_TYPE_PRINT_ENV:
        nbpps->decreaseCount();
        if ((err = processInputPrintEnv(cmds.getFirstCommand())) && (err != nil)) {
            cerr << err;
        }
        break;
    case COMMAND_TYPE_SET_ENV:
        nbpps->decreaseCount();
        if ((err = processInputSetEnv(cmds.getFirstCommand(), uManagement)) && (err != nil)) {
            cerr << err;
        }
        break;
    case COMMAND_TYPE_SERVER:
        nbpps->decreaseCount();
        if ((err = processInputServer(cmds.getFirstCommand(), sqn, uManagement)) && (err != nil)) {
            cerr << err;
        }
        break;
    default:
        bool isPipe = false;
        for (auto cmd : cmds.cmds) {
            // decrement the pipe counts and release invalid pipes
            nbpps->decreaseCount();

            // check if there exist input from pipe, if so, process it
            int pipeIndex = nbpps->getPipeWithCountIsZero();
            if (pipeIndex != NOT_FOUND) {
                isPipe = true;
                if ((err = processInputMain(cmd, nbpps->getPipeWithIndex(pipeIndex).readFd, nbpps->getPipeWithIndex(pipeIndex).writeFd, sqn, cmds.getOriginalInput(), nbpps, uManagement)) && (err != nil)) {
                    cerr << err;
                    break;
                }
            }

            // if there is no input from pipe, process the command as normal
            if (!isPipe) {
                if ((err = processInputMain(cmd, NOT_FOUND, NOT_FOUND, sqn, cmds.getOriginalInput(), nbpps, uManagement)) && (err != nil)) {
                    cerr << err;
                    break;
                }
            }
            nbpps->removeUsedPipes();

            // remove used user fifo
            uManagement->removeUsedUserFifo();
        }
    }

    // remove used pipes to avoid releasing user pipes that has been used
    nbpps->removeUsedPipes();
}

void processClientConnection(socketFd clientSocket, int sqn, usersManagement *uManagement, sharedMemory *shm) {
    int std[3] = { dup(STDOUT_FILENO), dup(STDERR_FILENO), dup(STDIN_FILENO) };
    dup2(clientSocket, STDOUT_FILENO);
    dup2(clientSocket, STDERR_FILENO);
    dup2(clientSocket, STDIN_FILENO);
    close(clientSocket);

    error err = nil;
    string input;

    char buffer[BUFFER_SIZE];
    if (error err = uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_LOGIN, uManagement->users[uManagement->getUserIndexBySqn(sqn)].name, uManagement->users[uManagement->getUserIndexBySqn(sqn)].ip, uManagement->users[uManagement->getUserIndexBySqn(sqn)].port)); err != nil) {
        cerr << err << "\n";
        goto RETURN;
    }

    while (true) {
        uManagement->sendPromptMessage();
        if (!getAndCheckIsExitAndEof(input)) {
            processInput(input, &uManagement->users[uManagement->getUserIndexBySqn(sqn)].nbpps, sqn, uManagement);
        } else {
            string name = uManagement->users[uManagement->getUserIndexBySqn(sqn)].name;

            dup2(std[0], STDOUT_FILENO);
            dup2(std[1], STDERR_FILENO);
            dup2(std[2], STDIN_FILENO);

            if (error err = uManagement->userLogout(sqn); err != nil) {
                cerr << err << "\n";
            } else {
                if (error err = uManagement->broadcastMessage(uManagement->serverMsgf(buffer, SERVER_MESSAGE_LOGOUT, name.c_str())); err != nil) {
                    cerr << err << "\n";
                }
            }
            break;
        }
    }

RETURN:
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    close(STDIN_FILENO);
    close(std[0]);
    close(std[1]);
    close(std[2]);
}

#endif
