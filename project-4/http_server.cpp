#include "http.hpp"

int main(int argc, char* argv[]) {
    string port = DEFAULT_PORT;
    if (argc == 2) {
        port = argv[1];
    }

    try {
        boost::asio::io_context ioContext;
        server server(ioContext, stoi(port));
        ioContext.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}