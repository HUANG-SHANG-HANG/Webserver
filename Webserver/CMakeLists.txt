cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(server
    src/main.cpp
    src/http/HttpConn.cpp
    src/log/Log.cpp
    src/timer/Timer.cpp
)

target_link_libraries(server pthread)

