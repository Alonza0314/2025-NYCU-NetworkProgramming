#include "np_simple.hpp"

void handle_sigchld(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    setenv("PATH", "bin:.", 1);

    signal(SIGCHLD, handle_sigchld);

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

    while (true) {
        socketFd clientSocket = serverSocket.acceptTcpServerSocket();
        if (clientSocket == CLIENT_DISCONNECTED) {
            cerr << "Client disconnected" << "\n";
            continue;
        }
        if (clientSocket == CLIENT_ERROR) {
            cerr << "Client error" << "\n";
            continue;
        }
        cout << "Client connected: " << clientSocket << "\n";

        pid_t pid;
        while ((pid = fork()) == -1) {
            waitpid(-1, NULL, 0);
        }
        if (pid == 0) {
            processClientConnection(&serverSocket, clientSocket);
        } else {
            serverSocket.closeClientSocket(clientSocket);
        }
    }

    return 0;
}