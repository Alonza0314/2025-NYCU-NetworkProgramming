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

error processInputSetEnv(command cmd) {
    if (cmd.args.size() != 3) {
        return ERROR_INVALID_NUMBER_OF_ARGUMENTS;
    }

    string var = cmd.args[1];
    string value = cmd.args[2];

    setenv(var.c_str(), value.c_str(), 1);

    return nil;
}

error processInputMain(command cmd, int readFd, int writeFd, numberedPipes *nbpps) {
    int pipeFd[2] = {-1, -1};
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
            if (pipe(pipeFd) < 0) {
                return ERROR_PIPE_FAILED;
            }
        } else {
            pipeFd[1] = outputToPipeFd;
        }
    }

    pid_t pid;
    // if fork() failed, wait for any one child process to finish
    while ((pid = fork()) == -1) {
        waitpid(-1, NULL, 0);
    }
    if (pid == 0) {
        // child process
        if (!cmd.isFileRedirect()) {
            if (cmd.isPipe()) {
                dup2(pipeFd[1], STDOUT_FILENO);

                if (cmd.isPipeNumberedErr()) {
                    dup2(pipeFd[1], STDERR_FILENO);
                }

                close(pipeFd[0]);
                close(pipeFd[1]);
            }
        } else {
            int fd = open(cmd.outputFile.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0777);
            if (fd < 0) {
                return ERROR_OPEN_FILE_FAILED;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (readFd != NOT_FOUND) {
            dup2(readFd, STDIN_FILENO);
            close(readFd);
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
        if (writeFd != NOT_FOUND) {
            close(writeFd);
        }
        if (readFd != NOT_FOUND) {
            close(readFd);
        }

        if (!cmd.isFileRedirect() && cmd.isPipe()) {
            if (outputToPipeFd == NOT_FOUND) {
                nbpps->addPipe(numberedPipe(pipeFd[0], pipeFd[1], cmd.pipeNum));
            }
        }

        if (!cmd.isPipe()) {
            if (waitpid(pid, nil, 0) < 0) {
                return ERROR_WAITPID_FAILED;
            }
        }
    }
    return nil;
}

void processInput(string input, numberedPipes *nbpps) {
    error err = nil;
    vector<command> commands = parseCommands(input);
    if (commands.empty() || commands[0].args.empty()) {
        return;
    }

    switch (getCommandType(commands[0].args[0])) {
    case COMMAND_TYPE_PRINT_ENV:
        nbpps->decreaseCount();
        nbpps->releaseInvalidPipes();
        if ((err = processInputPrintEnv(commands[0])) && (err != nil)) {
            cerr << err;
        }
        break;
    case COMMAND_TYPE_SET_ENV:
        nbpps->decreaseCount();
        nbpps->releaseInvalidPipes();
        if ((err = processInputSetEnv(commands[0])) && (err != nil)) {
            cerr << err;
        }
        break;
    default:
        bool isPipe = false;
        for (auto cmd : commands) {
            // decrement the pipe counts and release invalid pipes
            nbpps->decreaseCount();
            nbpps->releaseInvalidPipes();

            // check if there exist input from pipe, if so, process it
            int pipeIndex = nbpps->getPipeWithCountIsZero();
            if (pipeIndex != NOT_FOUND) {
                isPipe = true;
                if ((err = processInputMain(cmd, nbpps->getPipeWithIndex(pipeIndex).readFd, nbpps->getPipeWithIndex(pipeIndex).writeFd, nbpps)) && (err != nil)) {
                    cerr << err;
                }
            }

            // if there is no input from pipe, process the command as normal
            if (!isPipe) {
                if ((err = processInputMain(cmd, NOT_FOUND, NOT_FOUND, nbpps)) && (err != nil)) {
                    cerr << err;
                }
            }
        }
    }
}

void processClientConnection(tcpServerSocket *serverSocket, socketFd clientSocket) {
    dup2(clientSocket, STDOUT_FILENO);
    dup2(clientSocket, STDERR_FILENO);
    dup2(clientSocket, STDIN_FILENO);
    serverSocket->closeSocket();
    serverSocket->closeClientSocket(clientSocket);

    numberedPipes nbpps;

    string input;
    while (true) {
        cout << "% ";

        if (getAndCheckIsExitAndEof(input)) {
            break;
        }

        processInput(input, &nbpps);
    }

    while (wait(nil) > 0);

    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    close(STDIN_FILENO);

    exit(0);
}

#endif
