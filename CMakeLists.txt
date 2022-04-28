cmake_minimum_required(VERSION 3.20)
project(web_server)

set(CMAKE_CXX_STANDARD 14)

include(${CMAKE_CURRENT_SOURCE_DIR}/web_params/web_params.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/handler/handler.cmake)

add_executable(${PROJECT_NAME}
        server.cpp
        ${WEB_PARAMS}
        ${HANDLER})
