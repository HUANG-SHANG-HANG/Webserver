#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <string>

class HttpConn {
public:
    void parseRequest(const char* buf);
    std::string getResponse(const std::string& staticDir);

    std::string method_;
    std::string url_;
    std::string version_;
};

#endif

