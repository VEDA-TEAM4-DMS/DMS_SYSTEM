cmake_minimum_required(VERSION 3.10)
project(DMS_SYSTEM LANGUAGES CXX)

# 선택적으로 빌드할 수 있게 옵션 추가
option(BUILD_CLIENT "Build the client application" ON)
option(BUILD_SERVER "Build the server application" ON)

if(BUILD_CLIENT)
    add_subdirectory(src/client)
endif()

if(BUILD_SERVER)
    add_subdirectory(src/server)
endif()
