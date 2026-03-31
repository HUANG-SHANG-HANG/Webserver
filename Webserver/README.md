# 基于 Reactor 模型的高并发 C++ 网络服务器

## 项目简介

基于 Linux 平台实现的轻量级高并发 HTTP 服务器，采用 Reactor 事件驱动模型，支持多客户端并发访问。

## 技术栈

- C++17
- Linux
- epoll (ET模式 + EPOLLONESHOT)
- 线程池
- HTTP 请求解析
- 最小堆定时器
- 同步日志

## 项目架构
主线程 (Reactor)
├── epoll-wait 监听事件
├── 新连接 → accept → 注册到epoll + 定时器
└── 可读事件 → 提交到线程池
└── 工作线程
├── 读取请求
├── 解析HTTP (GET)
├── 返回静态文件 / 404
└── 关闭连接 + 移除定时器


## 核心模块

| 模块     | 文件                        | 说明                   |
|----------|-----------------------------|------------------------|
| 主程序   | src/main.cpp                | Reactor事件循环        |
| HTTP处理 | src/http/HttpConn.h/cpp     | 请求解析+响应生成      |
| 线程池   | src/threadpool/ThreadPool.h | 基于condition_variable |
| 定时器   | src/timer/Timer.h/cpp       | 最小堆实现超时回收     |
| 日志     | src/log/Log.h/cpp           | 同步日志,支持文件+终端 |

## 编译运行

```bash
cd build
cmake ..
make
cd ..
./build/server
## 浏览器访问: http://服务器IP:8080/

## 压力测试
测试环境: 4核4G 腾讯云轻量服务器

并发数	QPS  	平均延迟	最大延迟
100	    5857	25.37ms	    111.91ms
500  	5965	49.22ms 	1.74s
测试命令:

bash
wrk -t4 -c100 -d10s http://127.0.0.1:8080/
wrk -t4 -c500 -d10s http://127.0.0.1:8080/
## 项目目录
WebServer/
├── CMakeLists.txt
├── README.md
├── static/
│   └── index.html
└── src/
    ├── main.cpp
    ├── http/
    │   ├── HttpConn.h
    │   └── HttpConn.cpp
    ├── threadpool/
    │   └── ThreadPool.h
    ├── timer/
    │   ├── Timer.h
    │   └── Timer.cpp
    └── log/
        ├── Log.h
        └── Log.cpp
:注: build/ 和 server.log 为运行时生成，不纳入版本管理。
