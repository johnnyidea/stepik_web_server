cmake_minimum_required(VERSION 2.8)
project(final)

set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")

#include(${CMAKE_CURRENT_SOURCE_DIR}/web_params/web_params.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/handler/handler.cmake)

add_executable(${PROJECT_NAME}
        server.cpp
#        ${WEB_PARAMS}
#        ${HANDLER}
        )




