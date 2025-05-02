#ifndef MODEL_WIN_HPP
#define MODEL_WIN_HPP

#include "headerWin.hpp"
#include "constWin.hpp"

struct envValue {
    envKey requestMethod = "";
    envKey requestUri = "";
    envKey queryString = "";
    envKey serverProtocol = "";
    envKey httpHost = "";
    envKey serverAddr = "";
    envKey serverPort = "";
    envKey remoteAddr = "";
    envKey remotePort = "";
    envKey pathInfo = "";

    void print() {
        cout << "REQUEST_METHOD: " << requestMethod << endl;
        cout << "REQUEST_URI: " << requestUri << endl;
        cout << "QUERY_STRING: " << queryString << endl;
        cout << "SERVER_PROTOCOL: " << serverProtocol << endl;
        cout << "HTTP_HOST: " << httpHost << endl;
        cout << "SERVER_ADDR: " << serverAddr << endl;
        cout << "SERVER_PORT: " << serverPort << endl;
        cout << "REMOTE_ADDR: " << remoteAddr << endl;
        cout << "REMOTE_PORT: " << remotePort << endl;
        cout << "PATH_INFO: " << pathInfo << endl;
    }
};

struct clientInfo {
    string host = "";
    string port = "";
    string file = "";
};

struct htmlCreator {
    clientInfo infos[MAX_CLIENTS];

    static htmlCreator& getInstance() {
        static htmlCreator instance;
        return instance;
    }

    string getPanel() {
        const string formMethod = HTTP_METHOD_GET;
        const string formAction = HTTP_PATH_CONSOLE;
        string contentHead = PANEL_HEAD;

        string contentBodyFront = (boost::format(PANEL_BODY_FRONT) % formAction % formMethod).str();

        string hostMenu;
        for (int i = 0; i < MAX_SERVERS; i++) {
            hostMenu += (boost::format(PANEL_HOST_MENU) % (i + 1) % DOMAIN).str();
        }

        string testCaseMenu;
        for (int i = 0; i < 5; i++) {
            testCaseMenu += (boost::format(PANEL_TEST_CASE_MENU) % (i + 1)).str();
        }

        string contentBodyMiddle;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            contentBodyMiddle += (boost::format(PANEL_BODY_MIDDLE) % (i + 1) % i % hostMenu % DOMAIN % testCaseMenu).str();
        }

        string contentBodyEnd = PANEL_BODY_END;

        return HTTP_CONTENT_TYPE + contentHead + contentBodyFront + contentBodyMiddle + contentBodyEnd;
    }

    string getConsole() {
        string contentHead = CONSOLE_HEAD;

        string contentBodyFront = CONSOLE_BODY_FRONT;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (htmlCreator::getInstance().infos[i].host == "") {
                break;
            }
            contentBodyFront += (boost::format(CONSOLE_BODY_FRONT_INSIDE) % htmlCreator::getInstance().infos[i].host % htmlCreator::getInstance().infos[i].port).str();
        }

        string contentBodyMiddle = CONSOLE_BODY_MIDDLE;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (htmlCreator::getInstance().infos[i].host == "") {
                break;
            }
            contentBodyMiddle += (boost::format(CONSOLE_BODY_MIDDLE_INSIDE) % i).str();
        }

        string contentBodyEnd = CONSOLE_BODY_END;

        return HTTP_CONTENT_TYPE + contentHead + contentBodyFront + contentBodyMiddle + contentBodyEnd;
    }
};

struct Client : public enable_shared_from_this<Client> {
    int idx;
    tcp::socket tcpSocket;
    shared_ptr<tcp::socket> webSocket;
    tcp::resolver resolver;
    fstream file;
    char buffer[BUFFER_SIZE];

    Client(int index, boost::asio::io_context &io_context, shared_ptr<tcp::socket> socket)
        : idx(index), tcpSocket(io_context), webSocket(socket), resolver(io_context) {}

    void start() {
        file.open((TEST_CASE_DIR + htmlCreator::getInstance().infos[idx].file), ios::in); // Open file
        doResolve();
    }

    void doResolve() {
        auto self(shared_from_this());
        resolver.async_resolve(
            htmlCreator::getInstance().infos[idx].host,
            htmlCreator::getInstance().infos[idx].port,
            [this, self](boost::system::error_code ec, tcp::resolver::iterator it) {
                if (!ec) {
                    doConnect(it);
                }
            }
        );
    }

    void doConnect(tcp::resolver::iterator it) {
        auto self(shared_from_this());
        tcpSocket.async_connect(
            *it,
            [this, self](boost::system::error_code ec) {
                if (!ec) {
                    doRead();
                }
            }
        );
    }

    void doRead() {
        auto self(shared_from_this());
        tcpSocket.async_read_some(
            boost::asio::buffer(buffer, BUFFER_SIZE),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    string content(buffer, length);
                    memset(buffer, '\0', BUFFER_SIZE);

                    string output = outputShell(content);
                    writeToWeb(output);

                    if (content.find("% ") != string::npos) {
                        writeToNPServer();
                    } else {
                        doRead();
                    }
                }
            }
        );
    }

    void writeToNPServer() {
        auto self(shared_from_this());
        string command = getCommand();
        boost::asio::async_write(
            tcpSocket,
            boost::asio::buffer(command.c_str(), command.length()),
            [this, self](boost::system::error_code ec, size_t) {
                if (!ec) {
                    doRead();
                }
            }
        );
    }

    void writeToWeb(string content) {
        auto self(shared_from_this());
        boost::asio::async_write(
            *webSocket,
            boost::asio::buffer(content.c_str(), content.size()),
            [this, self](boost::system::error_code ec, size_t) {
                if (ec) {
                    cout << "Error writing: " << ec.message() << endl;
                }
            }
        );
    }

    string getCommand() {
        string command;
        if (file.is_open()) {
            getline(file, command);
            if (command.find("exit") != string::npos) {
                file.close();
            }
            command += "\n";
            string output = outputCommand(command);
            writeToWeb(output);
        }
        return command;
    }

    string htmlReplace(string content) {
        string contentReplace;
        for (int i = 0; i < (int)content.length(); i++) {
            switch (content[i]) {
            case '&':
                contentReplace += "&amp;";
                break;
            case '\"':
                contentReplace += "&quot;";
                break;
            case '\'':
                contentReplace += "&apos;";
                break;
            case '<':
                contentReplace += "&lt;";
                break;
            case '>':
                contentReplace += "&gt;";
                break;
            case '\n':
                contentReplace += "<br>";
                break;
            case '\r':
                contentReplace += "";
                break;
            case ' ':
                contentReplace += "&nbsp;";
                break;
            default:
                contentReplace += content[i];
                break;
            }
        }
        return contentReplace;
    }

    string outputShell(string content) {
        string contentEsc = htmlReplace(content);
        return (boost::format(SCRIPT_OUTPUT_SHELL) % idx % contentEsc).str();
    }

    string outputCommand(string content) {
        string contentEsc = htmlReplace(content);
        return (boost::format(SCRIPT_OUTPUT_COMMAND) % idx % contentEsc).str();
    }
};

struct Session : public enable_shared_from_this<Session> {
    tcp::socket tcpSocket;
    boost::asio::io_context &ioCtx;
    char buffer[BUFFER_SIZE];
    envValue env;

    Session(tcp::socket socket, boost::asio::io_context &io_context) : tcpSocket(move(socket)), ioCtx(io_context) {}

    void start() {
        doRead();
    }

    void doRead() {
        auto self(shared_from_this());
        tcpSocket.async_read_some(
            boost::asio::buffer(buffer, BUFFER_SIZE),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    parseHTTPRequest();
                    doWrite(HTTP_200_HEADER);
                    if (env.pathInfo == HTTP_PATH_PANEL) {
                        string panel = htmlCreator::getInstance().getPanel();
                        doWrite(panel);
                    } else if (env.pathInfo == HTTP_PATH_CONSOLE) {
                        parseQueryString();
                        string console = htmlCreator::getInstance().getConsole();
                        doWrite(console);
                        makeConnection();
                    }
                }
            }
        );
    }

    void doWrite(string httpContent) {
        auto self(shared_from_this());
        boost::asio::async_write(
            tcpSocket,
            boost::asio::buffer(httpContent.c_str(), httpContent.size()),
            [this, self](boost::system::error_code ec, size_t) {
                if (ec) {
                    cout << "Error writing: " << ec.message() << endl;
                }
            }
        );
    }

    void parseHTTPRequest() {
        stringstream ss(buffer);
        ss >> env.requestMethod >> env.requestUri >> env.serverProtocol;

        string temp;
        ss >> temp;
        if (temp == HTTP_HOST_HEADER) {
            ss >> env.httpHost;
        }

        size_t pos = env.requestUri.find("?");
        if (pos != string::npos) {
            env.queryString = env.requestUri.substr(pos + 1);
            env.pathInfo = env.requestUri.substr(0, pos);
        } else {
            env.pathInfo = env.requestUri;
        }

        env.serverAddr = tcpSocket.local_endpoint().address().to_string();
        env.serverPort = to_string(tcpSocket.local_endpoint().port());
        env.remoteAddr = tcpSocket.remote_endpoint().address().to_string();
        env.remotePort = to_string(tcpSocket.remote_endpoint().port());
    
        env.print();
    }

    void parseQueryString() {
        vector<string> tmp;
        boost::split(tmp, env.queryString, boost::is_any_of("&"));
        for (unsigned long int i = 0; i < tmp.size(); i++) {
            vector<string> tmp2;
            boost::split(tmp2, tmp[i], boost::is_any_of("="));
            if (tmp2.size() == 2) {
                switch (tmp2[0][0]) {
                case 'h':
                    htmlCreator::getInstance().infos[i / 3].host = tmp2[1];
                    break;
                case 'p':
                    htmlCreator::getInstance().infos[i / 3].port = tmp2[1];
                    break;
                case 'f':
                    htmlCreator::getInstance().infos[i / 3].file = tmp2[1];
                    break;
                }
            }
        }
    }

    void makeConnection() {
        shared_ptr<tcp::socket> webSocket = make_shared<tcp::socket>(move(tcpSocket));

        for (int idx = 0; idx < MAX_CLIENTS; idx++) {
            if (htmlCreator::getInstance().infos[idx].host == "") {
                return;
            }
            make_shared<Client>(idx, ioCtx, webSocket)->start();
        }
    }
};

struct Server {
    tcp::acceptor tcpAcceptor;
    boost::asio::io_context &ioCtx;

    Server(boost::asio::io_context &io_context, short port)
        : tcpAcceptor(io_context, tcp::endpoint(tcp::v4(), port)), ioCtx(io_context) {
        doAccept();
    }

    void doAccept() {
        tcpAcceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    make_shared<Session>(move(socket), ioCtx)->start();
                }
                doAccept();
            }
        );
    }
};

#endif
