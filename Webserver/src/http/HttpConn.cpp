#include "http/HttpConn.h"
#include <cstdio>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

void HttpConn::parseRequest(const char* buf) {
    // 只解析请求行第一行: GET /index.html HTTP/1.1
    std::istringstream stream(buf);
    stream >> method_ >> url_ >> version_;

    // 默认访问 /index.html
    if (url_ == "/") {
        url_ = "/index.html";
    }
}

std::string HttpConn::getResponse(const std::string& staticDir) {
    std::string filePath = staticDir + url_;

    // 检查文件是否存在
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0 || S_ISDIR(fileStat.st_mode)) {
        std::string body = "<h1>404 Not Found</h1>";
        std::string resp;
        resp += "HTTP/1.1 404 Not Found\r\n";
        resp += "Content-Type: text/html\r\n";
        resp += "Connection: close\r\n";
        resp += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        resp += "\r\n";
        resp += body;
        return resp;
    }

    // 读取文件内容
    std::ifstream ifs(filePath, std::ios::binary);
    std::string body((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());

    std::string resp;
    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Content-Type: text/html\r\n";
    resp += "Connection: close\r\n";
    resp += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    resp += "\r\n";
    resp += body;
    return resp;
}

