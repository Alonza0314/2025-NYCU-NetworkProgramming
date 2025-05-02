#ifndef CONST_HPP
#define CONST_HPP

#include "header.hpp"

#define error char*
#define nil nullptr
#define envKey string

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

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

const string HTTP_200_HEADER = "HTTP/1.1 200 OK\r\n";
const string HTTP_CONTENT_TYPE = "Content-Type: text/html\r\n\r\n";

const string HTML_FRAME = "\
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
                background-color: #212529;\
            }\
            pre {\
                color: #cccccc;\
            }\
            b {\
                color: #01b468;\
            }\
            </style>\
        </head>\
        <body>\
            <table class=\"table table-dark table-bordered\">\
                <thead>\
                    <tr id=\"table_head\">\
                    </tr>\
                </thead>\
                <tbody>\
                    <tr id=\"table_body\">\
                    </tr>\
                </tbody>\
            </table>\
        </body>\
    </html>";
const string HTML_TABLE_FIRST_PART = "<script>document.querySelector('#table_head').innerHTML += '<th scope=\\\"col\\\">%s</th>';</script>";
const string HTML_TABLE_SECOND_PART = "<script>document.querySelector('#table_body').innerHTML += '<td><pre id=\\\"user_%s\\\" class=\\\"mb-0\\\"></pre></td>';</script>";
const string HTML_IS_COMMAND = "<script>document.querySelector('#user_%s').innerHTML += '<b>%s</b>';</script>";
const string HTML_NOT_COMMAND = "<script>document.querySelector('#user_%s').innerHTML += '%s';</script>";

const string TEST_CASE_DIR = "./test_case/";

const string SYSTEM_PROMPT = "% ";
const string SYSTEM_EXIT = "exit\n";

#endif
