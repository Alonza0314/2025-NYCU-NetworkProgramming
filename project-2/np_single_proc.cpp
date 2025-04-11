#include "np_single_proc.hpp"

void handle_sigchld(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigint(int signum) {
    cout << "\nReceived SIGINT, shutting down server...";
    exit(0);    
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, handle_sigchld);
    signal(SIGINT, handle_sigint);

    int port = PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    tcpServerSocket serverSocket;
    if (error err = serverSocket.newTcpServerSocket(port); err != nil) {
        cerr << err << endl;
        return 1;
    }
    if (error err = serverSocket.listenTcpServerSocket(); err != nil) {
        cerr << err << endl;
        return 1;
    }

    socketFd maxFd = FD_SETSIZE;
    fd_set masterSet, workingSet;
    FD_ZERO(&masterSet);
    FD_SET(serverSocket.getServerSocket(), &masterSet);

    usersManagement uManagement;
    userPipes uspps;

    cout << "Server started...\n";
    while (true) {
        workingSet = masterSet;

        socketFd maxSocketFd = serverSocket.getServerSocket();
        for (socketFd i = 0; i < maxFd; i++) {
            if (FD_ISSET(i, &masterSet)) {
                maxSocketFd = max(maxSocketFd, i);
            }
        }

        if (select(maxSocketFd + 1, &workingSet, NULL, NULL, NULL) < 0) {
            if (errno != EINTR) {
                cerr << "Select error\n";
                continue;
            }
            continue;
        }

        if (FD_ISSET(serverSocket.getServerSocket(), &workingSet)) {
            sockaddr_in clientAddress;
            socketFd clientSocket = serverSocket.acceptTcpServerSocket(clientAddress);
            cout << "clientSocket: " << clientSocket << "\n";
            if (clientSocket == CLIENT_DISCONNECTED) {
                cerr << "Client disconnected" << "\n";
            } else if (clientSocket == CLIENT_ERROR) {
                cerr << "Client error" << "\n";
            } else {
                FD_SET(clientSocket, &masterSet);

                if (error err = uManagement.userLogin(clientSocket, clientAddress); err != nil) {
                    cerr << err << "\n";
                }
            }
        }

    CHECK_CLIENT:
        for (socketFd fd = 0; fd < maxFd; fd++) {
            if (fd != serverSocket.getServerSocket() && FD_ISSET(fd, &workingSet)) {
                if (error err = processClientConnection(fd, &uManagement, &uspps); err != nil) {
                    if (err == ERROR_USER_DISCONNECTED) {
                        int userSqn = uManagement.getUserSqnByClientSocket(fd);
                        if (error err = uManagement.userLogout(fd); err != nil) {
                            cerr << err << "\n";
                        } else {
                            if (userSqn != NOT_FOUND) {
                                uspps.removeUserAccordingPipes(userSqn);
                            }
                        }
                        FD_CLR(fd, &masterSet);
                    } else {
                        cerr << err << "\n";
                    }
                }
            }
        }
    }

    return 0;
}