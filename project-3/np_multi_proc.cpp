#include "np_multi_proc.hpp"

volatile sig_atomic_t serverRunning = 1;
tcpServerSocket serverSocket;
usersManagement* uManagement = nil;
sharedMemory shm;

void handle_sigChild(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigInt(int signum) {
    cout << "\nReceived SIGINT, shutting down server...\n";
    serverRunning = 0;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    if (error err = shm.detachShm(&uManagement); err != nil) {
        cerr << err << "\n";
    }
    if (error err = shm.deleteShm(); err != nil) {
        cerr << err << "\n";
    }

    sleep(1);
    exit(0);
}

void handle_broadcast(int signum) {
    cout << uManagement->getBroadcastBuffer() << flush;
}

void handle_openUserFifo(int signum) {
    for (int i = 0; i < MAX_CLIENTS_USERS; i++) {
        if (uManagement->userFifos[i].writeFdNotOpenButBusy()) {
            uManagement->userFifos[i].openReadFd();
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, handle_sigChild);
    signal(SIGINT, handle_sigInt);
    signal(SIGUSR1, handle_broadcast);
    signal(SIGUSR2, handle_openUserFifo);

    setenv("PATH", "bin:.", 1);

    int port = PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    if (error err = serverSocket.newTcpServerSocket(port); err != nil) {
        cerr << err << endl;
        return 1;
    }
    if (error err = serverSocket.listenTcpServerSocket(); err != nil) {
        cerr << err << endl;
        return 1;
    }

    if (error err = shm.createShm(); err != nil) {
        cerr << err << "\n";
        return 1;
    }

    if (error err = shm.attachShm(&uManagement); err != nil) {
        cerr << err << "\n";
        return 1;
    }

    cout << "Server started...\n";
    while (serverRunning) {
        sockaddr_in clientAddress;
        socketFd clientSocket = serverSocket.acceptTcpServerSocket(clientAddress);
        cout << "clientSocket: " << clientSocket << "\n";
        int sqn = NOT_FOUND;
        if (clientSocket == CLIENT_DISCONNECTED) {
            cerr << "Client disconnected" << "\n";
            continue;
        } else if (clientSocket == CLIENT_ERROR) {
            cerr << "Client error" << "\n";
            continue;
        } else {
            if (error err = uManagement->userLogin(clientSocket, clientAddress, &sqn); err != nil) {
                cerr << err << "\n";
                return 1;
            }
        }

        pid_t pid;
        while ((pid = fork()) == -1) {
            waitpid(-1, NULL, 0);
        }
        if (pid == 0) {
            // child process, for connection
            serverSocket.closeSocket();
            uManagement->setUserPid(sqn, getpid());
            if (error err = uManagement->sendWelcomeMessage(clientSocket); err != nil) {
                cerr << err << "\n";    
                goto EXIT_CLIENT;
            }
            processClientConnection(clientSocket, sqn, uManagement, &shm);
        EXIT_CLIENT:
            uManagement->removeUserRelatedFifo(sqn);
            if (error err = shm.detachShm(&uManagement); err != nil) {
                cerr << err << "\n";
                exit(1);
            }
            exit(0);
        } else {
            close(clientSocket);
        }
    }

    return 0;
}