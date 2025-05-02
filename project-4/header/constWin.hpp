#ifndef CONST_HPP
#define CONST_HPP

#include "headerWin.hpp"

#define error char*
#define nil nullptr
#define envKey string

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5
#define MAX_SERVERS 12
#define DOMAIN "cs.nycu.edu.tw"

const string DEFAULT_PORT = "12345";

const envKey REQUEST_METHOD = "REQUEST_METHOD";
const envKey REQUEST_URI = "REQUEST_URI";
const envKey QUERY_STRING = "QUERY_STRING";
const envKey SERVER_PROTOCOL = "SERVER_PROTOCOL";
const envKey HTTP_HOST = "HTTP_HOST";
const envKey SERVER_ADDR = "SERVER_ADDR";
const envKey SERVER_PORT = "SERVER_PORT";
const envKey REMOTE_ADDR = "REMOTE_ADDR";
const envKey REMOTE_PORT = "REMOTE_PORT";
const envKey PATH_INFO = "PATH_INFO";

const string HTTP_METHOD_GET = "GET";
const string HTTP_ACTION_CONSOLE = "console.cgi";
const string HTTP_200_HEADER = "HTTP/1.1 200 OK\r\n";
const string HTTP_CONTENT_TYPE = "Content-Type: text/html\r\n\r\n";
const string HTTP_PATH_PANEL = "/panel.cgi";
const string HTTP_PATH_CONSOLE = "/console.cgi";
const string HTTP_HOST_HEADER = "Host:";

const string PANEL_HEAD = "\
            <!DOCTYPE html>\
            <html lang=\"en\">\
            <head>\
                <title>NP Project 4 Panel</title>\
                <link\
                rel=\"stylesheet\"\
                href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\
                integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\
                crossorigin=\"anonymous\"\
                />\
                <link\
                href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\
                rel=\"stylesheet\"\
                />\
                <link\
                rel=\"icon\"\
                type=\"image/png\"\
                href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"\
                />\
                <style>\
                * {\
                    font-family: 'Source Code Pro', monospace;\
                }\
                </style>\
            </head>\
            <body class=\"bg-secondary pt-5\">";
const string PANEL_BODY_FRONT = "\
                <form action=\"%1%\" method=\"%2%\">\
                <table class=\"table mx-auto bg-light\" style=\"width: inherit\">\
                    <thead class=\"thead-dark\">\
                    <tr>\
                        <th scope=\"col\">#</th>\
                        <th scope=\"col\">Host</th>\
                        <th scope=\"col\">Port</th>\
                        <th scope=\"col\">Input File</th>\
                    </tr>\
                    </thead>\
                    <tbody>";
const string PANEL_HOST_MENU = "\
                            <option value=\"nplinux%1%.%2%\">nplinux%1%</option>";
const string PANEL_TEST_CASE_MENU = "\
                            <option value=\"t%1%.txt\">t%1%.txt</option>";
const string PANEL_BODY_MIDDLE = "\
                    <tr>\
                        <th scope=\"row\" class=\"align-middle\">Session %1%</th>\
                        <td>\
                        <div class=\"input-group\">\
                            <select name=\"h%2%\" class=\"custom-select\">\
                            <option></option>%3%\
                            </select>\
                            <div class=\"input-group-append\">\
                            <span class=\"input-group-text\">.%4%</span>\
                            </div>\
                        </div>\
                        </td>\
                        <td>\
                        <input name=\"p%2%\" type=\"text\" class=\"form-control\" size=\"5\" />\
                        </td>\
                        <td>\
                        <select name=\"f%2%\" class=\"custom-select\">\
                            <option></option>\
                            %5%\
                        </select>\
                        </td>\
                    </tr>";
const string PANEL_BODY_END = "\
                    <tr>\
                        <td colspan=\"3\"></td>\
                        <td>\
                        <button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>\
                        </td>\
                    </tr>\
                    </tbody>\
                </table>\
                </form>\
            </body>\
            </html>";
const string CONSOLE_HEAD = "\
            <!DOCTYPE html>\
            <html lang=\"en\">\
            <head>\
                <meta charset=\"UTF-8\" />\
                <title>NP Project 4 Console</title>\
                <link\
                rel=\"stylesheet\"\
                href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\
                integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\
                crossorigin=\"anonymous\"\
                />\
                <link\
                href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\
                rel=\"stylesheet\"\
                />\
                <link\
                rel=\"icon\"\
                type=\"image/png\"\
                href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\
                />\
                <style>\
                * {\
                    font-family: 'Source Code Pro', monospace;\
                    font-size: 1rem !important;\
                }\
                body {\
                    background-color: #232731;\
                }\
                pre {\
                    color: #D8DEE9;\
                }\
                b {\
                    color: #a3be8c;\
                }\
                th {\
                    color: #81A1C1;\
                }\
                </style>\
            </head>";
const string CONSOLE_BODY_FRONT = "\
            <body>\
                <table class=\"table table-dark table-bordered\">\
                <thead>\
                    <tr>";
const string CONSOLE_BODY_FRONT_INSIDE = "\
                    <th scope=\"col\">%1%:%2%</th>";
const string CONSOLE_BODY_MIDDLE = "\
                    </tr>\
                </thead>\
                <tbody>\
                    <tr>";
const string CONSOLE_BODY_MIDDLE_INSIDE = "\
                    <td><pre id=\"s%1%\" class=\"mb-0\"></pre></td>";
const string CONSOLE_BODY_END = "\
                    </tbody>\
                </table>\
            </body>\
            </html>";

const string SCRIPT_OUTPUT_SHELL = "<script>document.getElementById('s%1%').innerHTML += '%2%';</script>";
const string SCRIPT_OUTPUT_COMMAND = "<script>document.getElementById('s%1%').innerHTML += '<b>%2%</b>';</script>";

const string TEST_CASE_DIR = "./test_case/";

const string SYSTEM_PROMPT = "% ";
const string SYSTEM_EXIT = "exit\n";

#endif
