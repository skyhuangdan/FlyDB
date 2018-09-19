cmake_minimum_required(VERSION 3.10)
project(flyDB)

set(CMAKE_CXX_STANDARD 11)

add_executable(flyDB main.cpp FlyServer.cpp FlyServer.h FlyDB.cpp FlyDB.h CommandEntry.cpp CommandEntry.h)