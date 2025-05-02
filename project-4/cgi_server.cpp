#include "cgi.hpp"

int main(int argc, char *argv[]) {
    string port = DEFAULT_PORT;
    if (argc == 2) {
        port = argv[1];
    }

    try {
        boost::asio::io_context io_context;
        Server s(io_context, std::atoi(port.c_str()));
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}