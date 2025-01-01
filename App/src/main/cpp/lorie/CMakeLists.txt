cmake_minimum_required(VERSION 3.4.1)
include_directories(inc)
include_directories(${CMAKE_SOURCE_DIR}/server/renderer)
include_directories(${CMAKE_SOURCE_DIR}/client/renderer)
include_directories(${CMAKE_SOURCE_DIR}/server/inc)
add_library(Xlorie
        SHARED
        android.cpp
        server.c
        client.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

target_link_libraries(Xlorie
        log
        EGL
        GLESv3
        android
        Server
        Client
)