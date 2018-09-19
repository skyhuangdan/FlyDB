cmake_minimum_required(VERSION 3.10)
project(flyDB)

set(CMAKE_CXX_STANDARD 11)

add_executable(flyDB main.cpp server/FlyServer.cpp server/FlyServer.h server/FlyDB.cpp server/FlyDB.h server/CommandEntry.cpp server/CommandEntry.h)