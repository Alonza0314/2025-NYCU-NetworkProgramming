#ifndef MODEL_HPP
#define MODEL_HPP

#include "header.hpp"
#include "const.hpp"
#include "util.hpp"

struct envValue {
    envKey requestMethod;
    envKey requestUri;
    envKey queryString;
    envKey serverProtocol;
    envKey httpHost;
    envKey serverAddr;
    envKey serverPort;
    envKey remoteAddr;
    envKey remotePort;
    
    void setEnv() {
        setenv(REQUEST_METHOD.c_str(), requestMethod.c_str(), 1);
        setenv(REQUEST_URI.c_str(), requestUri.c_str(), 1);
        setenv(QUERY_STRING.c_str(), queryString.c_str(), 1);
        setenv(SERVER_PROTOCOL.c_str(), serverProtocol.c_str(), 1);
        setenv(HTTP_HOST.c_str(), httpHost.c_str(), 1);
        setenv(SERVER_ADDR.c_str(), serverAddr.c_str(), 1);
        setenv(SERVER_PORT.c_str(), serverPort.c_str(), 1);
        setenv(REMOTE_ADDR.c_str(), remoteAddr.c_str(), 1);
        setenv(REMOTE_PORT.c_str(), remotePort.c_str(), 1);
    }

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
    }
};

struct session: public enable_shared_from_this<session> {
    tcp::socket tcpSocket;
    char  buffer[BUFFER_SIZE];
    envValue env;

    session(tcp::socket socket) : tcpSocket(move(socket)) {}

    void start() {
        doRead();
    }

    void doRead() {
        auto self(shared_from_this());
        tcpSocket.async_read_some(
            boost::asio::buffer(buffer, BUFFER_SIZE),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    string requestContent;
                    dataToMessage(buffer, requestContent, length);
                    clearBuffer(buffer);
                    char* argv[] = { nil };
                    httpParser(requestContent);

                    string targetFile;
                    pid_t pid;
                    while((pid = fork()) == -1) {
                        waitpid(-1, NULL, WNOHANG);
                    }
                    if (pid == 0) {
                        // child process
                        targetFile = "." + env.requestUri.substr(0, env.requestUri.find('?', 0));

                        env.setEnv();

                        dup2(tcpSocket.native_handle(), STDOUT_FILENO);
                        tcpSocket.close();

                        cout << HTTP_200_HEADER << flush;

                        execv(targetFile.c_str(), argv);
                        exit(0);
                    } else {
                        // parent process
                        tcpSocket.close();
                        waitpid(pid, NULL, WNOHANG);
                    }
                }
            }
        );
    }

    void httpParser(string requestContent) {
        int endOfFirstLine, endOfSecLine, start, end;

        endOfFirstLine = requestContent.find('\n', 0); // GET /panel.cgi HTTP/1.1
        endOfSecLine = requestContent.find('\n', endOfFirstLine + 1); // Host: npllinuxX.cs.nycu.edu.tw:12345
        string firstLine = requestContent.substr(0, endOfFirstLine - 1);
        string secLine = requestContent.substr(endOfFirstLine + 1, endOfSecLine - (endOfFirstLine + 2));
        
        // parse REQUEST_METHOD
        start = 0;
        end = firstLine.find(' ', 0);
        env.requestMethod = firstLine.substr(start, end - start);

        // parse REQUEST_URI and QUERY_STRING
        start = end + 1;
        // query string
        if((end = firstLine.find('?', start)) != -1) {
            end = firstLine.find('?', start);
            env.requestUri = firstLine.substr(start, end - start);

            start = end + 1;
            end = firstLine.find(' ', start);
            env.queryString = firstLine.substr(start, end - start);
            env.requestUri = env.requestUri + "?" + env.queryString;
        } else {
            end = firstLine.find(' ', start);
            env.requestUri = firstLine.substr(start, end - start);
            env.queryString = "";
        }

        // parse SERVER_PROTOCOL
        start = end + 1;
        env.serverProtocol = firstLine.substr(start);
        // parse HTTP_HOST
        secLine = secLine.substr(secLine.find(' ', 0) + 1);
        env.httpHost = secLine;
        // parse SERVER_ADDR
        env.serverAddr = tcpSocket.local_endpoint().address().to_string();
        // parse SERVER_PORT
        env.serverPort = secLine.substr(secLine.find(':', 0) + 1);
        // parse REMOTE_ADDR
        env.remoteAddr = tcpSocket.remote_endpoint().address().to_string();
        // parse REMOTE_PORT
        env.remotePort = to_string(htons(tcpSocket.remote_endpoint().port()));

        env.print();
    }
};

struct server {
    tcp::acceptor tcpAcceptor;

    server(boost::asio::io_context& ioContext, short port) : tcpAcceptor(ioContext, tcp::endpoint(tcp::v4(), port)) {
        doAccept();
    }

    void doAccept() {
        tcpAcceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    make_shared<session>(move(socket))->start();
                }
                doAccept();
            }
        );
    }
};

struct clientInfo {
    string host;
    string port;
    string file;
};

struct htmlCreator {
    clientInfo  infos[MAX_CLIENTS];

    static htmlCreator& getInstance() {
        static htmlCreator instance;
        return instance;
    }

    void parseString() {
        string envVal = getQuerystring() + "&";
        string clientData;
        int start = 0;
        int end;
        int index = 0;

        while ((end = envVal.find('&', start)) != -1) {
            clientData = envVal.substr(start, end - start);
            if (clientData.length() == 3) {
                break;
            }
            if (index % 3 == 0) {
                infos[index / 3].host = clientData.substr(3);
            } else if (index % 3 == 1) {
                infos[index / 3].port = clientData.substr(3);
            } else {
                infos[index / 3].file = clientData.substr(3);
            }
            index++;
            start = end + 1;
        }
    }

    void sendHTMLFrame() {
        cout << HTTP_CONTENT_TYPE << flush;
        cout << HTML_FRAME << flush;
    }

    void sendHTAMLTable(string index, string msg) {
        string firstPart, secondPart;
        htmlf(index, msg, &firstPart, &secondPart);
        cout << firstPart << flush;
        cout << secondPart << flush;
    }

    void sendMessage(string index, string message ,bool isCommand) {
        string parsedContent;
        for (int i = 0; i < (int)message.length(); i++) {
            switch (message[i]) {
            case '\n':
                parsedContent += "<br>";
                break;
            case '\r':
                parsedContent += "";
                break;
            case '\'':
                parsedContent += "\\'";
                break;
            case '<':
                parsedContent += "&lt;";
                break;
            case '>':
                parsedContent += "&gt;";
                break;
            case '&':
                parsedContent += "&amp;";
                break;
            default:
                parsedContent += message[i];
                break;
            }
        }

        string htmlMessage;
        commandf(index, parsedContent, isCommand, &htmlMessage);
        cout << htmlMessage << flush;
    }

    void htmlf(string index, string msg, string* firstPart, string* secondPart) {
        char buffer[BUFFER_SIZE];
        sprintf(buffer, HTML_TABLE_FIRST_PART.c_str(), msg.c_str());
        *firstPart = string(buffer);
        sprintf(buffer, HTML_TABLE_SECOND_PART.c_str(), index.c_str());
        *secondPart = string(buffer);
    }

    void commandf(string index, string parsedContent, bool isCommand, string* ret) {
        char buffer[BUFFER_SIZE];
        if (isCommand) {
            sprintf(buffer, HTML_IS_COMMAND.c_str(), index.c_str(), parsedContent.c_str());
        } else {
            sprintf(buffer, HTML_NOT_COMMAND.c_str(), index.c_str(), parsedContent.c_str());
        }
        *ret = string(buffer);
    }

    string getQuerystring() {
        return getenv(QUERY_STRING.c_str());
    }
};

struct connection: public enable_shared_from_this<connection> {
    tcp:: socket tcpSocket;
    tcp:: resolver tcpResolver;
    boost::asio::io_context& ioCtx;
    boost::asio::ip::tcp::resolver::results_type endpoints;
    string id;
    ifstream file;
    char buffer[BUFFER_SIZE];

    connection(boost::asio::io_context& ioCtx, string id) : tcpSocket(ioCtx), tcpResolver(ioCtx), ioCtx(ioCtx), id(id) {}

    void start() {
        doResolve();
    }

    void doResolve() {
        clientInfo info;
        info = htmlCreator::getInstance().infos[stoi(id)];

        auto self(shared_from_this());
        tcpResolver.async_resolve(
            info.host,
            info.port,
            [this, self](const boost::system::error_code& ec, const boost::asio::ip::tcp::resolver::results_type results) {
                if (!ec) {
                    clearBuffer(buffer);
                    endpoints = results;
                    doConnect();
                } else {
                    tcpSocket.close();
                }
            }
        );
    }

    void doConnect() {
        auto self(shared_from_this());
        boost::asio::async_connect(
            tcpSocket,
            endpoints,
            [this, self](const boost::system::error_code ec, const tcp::endpoint& endpoint) {
                if (!ec) {
                    clearBuffer(buffer);
                    clientInfo* infos = htmlCreator::getInstance().infos;

                    string filePath = TEST_CASE_DIR + infos[stoi(id)].file;
                    file.open(filePath);

                    doRead();
                } else {
                    tcpSocket.close();
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
                    string message;
                    dataToMessage(buffer, message, length);
                    clearBuffer(buffer);
                    htmlCreator::getInstance().sendMessage(id, message, false);

                    if (length != 0) {
                        if (message.find(SYSTEM_PROMPT) != string::npos) {
                            string command;
                            getline(file, command);
                            command += "\n";
                            htmlCreator::getInstance().sendMessage(id, command, true);
                            doWrite(command);
                        } else {
                            doRead();
                        }
                    }
                } else {
                    tcpSocket.close();
                }
            }
        );
    }

    void doWrite(string command) {
        auto self(shared_from_this());
        const char *msg = command.c_str();
        boost::asio::async_write(
            tcpSocket,
            boost::asio::buffer(msg, sizeof(char)*command.length()),
            [this, self, command](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    string strCmp(SYSTEM_EXIT);
                    if (command.compare(strCmp) == 0) {
                        tcpSocket.close();
                    } else {
                        doRead();
                    }
                }
            }
        );
    }
};

#endif
