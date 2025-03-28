#include "npshellHeader.hpp"

void handle_sigchld(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    setenv("PATH", "bin:.", 1);

    signal(SIGCHLD, handle_sigchld);

    numberedPipes nbpps;

    string input;
    while (true) {
        cout << "% ";

        if (getAndCheckIsExitAndEof(input)) {
            break;
        }

        processInput(input, &nbpps);
    }

    // wait for all child processes to finish
    while (wait(nil) > 0);

    return 0;
}