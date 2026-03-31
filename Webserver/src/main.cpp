#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "threadpool/ThreadPool.h"
#include "http/HttpConn.h"
#include "log/Log.h"
#include "timer/Timer.h"

const int MAX_EVENTS = 1024;
const int PORT = 8080;
const int TIMEOUT_MS = 60000;
const std::string STATIC_DIR = "./static";

Timer timer;

void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void addFd(int epollFd, int fd, bool isListen = false) {
    epoll_event ev;
    ev.data.fd = fd;
    if (isListen) {
        ev.events = EPOLLIN;
    } else {
        ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    }
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
    setNonBlocking(fd);
}

void handleAccept(int listenFd, int epollFd) {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd < 0) {
        LOG_ERROR("accept failed");
        return;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
    LOG_INFO("New connection fd=%d from %s:%d", clientFd, ip, ntohs(clientAddr.sin_port));

    addFd(epollFd, clientFd, false);

    {
        std::lock_guard<std::mutex> lock(timer.mtx);
        timer.add(clientFd, TIMEOUT_MS, [clientFd]() {
            LOG_INFO("Connection timeout, close fd=%d", clientFd);
            close(clientFd);
        });
    }
}

void handleRead(int fd, const std::string& staticDir) {
    char buf[4096] = {0};
    int n = read(fd, buf, sizeof(buf) - 1);

    if (n <= 0) {
        {
            std::lock_guard<std::mutex> lock(timer.mtx);
            timer.remove(fd);
        }
        close(fd);
        return;
    }

    HttpConn conn;
    conn.parseRequest(buf);
    LOG_INFO("fd=%d %s %s %s", fd, conn.method_.c_str(), conn.url_.c_str(), conn.version_.c_str());

    std::string response = conn.getResponse(staticDir);
    write(fd, response.c_str(), response.size());

    {
        std::lock_guard<std::mutex> lock(timer.mtx);
        timer.remove(fd);
    }
    close(fd);
}

int main() {
    Log::instance().init("server.log");

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    bind(listenFd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listenFd, 128);

    int epollFd = epoll_create(1);
    addFd(epollFd, listenFd, true);

    ThreadPool pool(4);

    LOG_INFO("Server started on port %d", PORT);
    LOG_INFO("Static dir: %s", STATIC_DIR.c_str());
    LOG_INFO("Timeout: %d ms", TIMEOUT_MS);

    epoll_event events[MAX_EVENTS];

    while (true) {
        int waitTime;
        {
            std::lock_guard<std::mutex> lock(timer.mtx);
            waitTime = timer.getNextTick();
        }

        int nReady = epoll_wait(epollFd, events, MAX_EVENTS, waitTime);

        {
            std::lock_guard<std::mutex> lock(timer.mtx);
            timer.tick();
        }

        for (int i = 0; i < nReady; i++) {
            int fd = events[i].data.fd;

            if (fd == listenFd) {
                handleAccept(listenFd, epollFd);
            } else if (events[i].events & EPOLLIN) {
                pool.addTask([fd]() {
                    handleRead(fd, STATIC_DIR);
                });
            }
        }
    }

    close(listenFd);
    close(epollFd);
    return 0;
}

