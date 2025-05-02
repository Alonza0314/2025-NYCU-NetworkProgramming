#include "http.hpp"

int main() {
    htmlCreator::getInstance().parseString();
    htmlCreator::getInstance().sendHTMLFrame();

    try {
        boost::asio::io_context io_context;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            clientInfo* infos = htmlCreator::getInstance().infos;
            if (infos[i].host == "") {
                break;
            }
            htmlCreator::getInstance().sendHTAMLTable(to_string(i), infos[i].host + ":" + infos[i].port);
            make_shared<connection>(io_context, to_string(i))->start();
        }
        io_context.run();
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}