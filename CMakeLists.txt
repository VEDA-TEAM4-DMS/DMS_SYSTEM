cmake_minimum_required(VERSION 3.10)
project(OSRMClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 실행 파일 이름과 소스 파일 목록 (osrm.cpp 포함!)
add_executable(server
    ./src/server.cpp
    ./src/osrm.cpp
)

add_executable(client
    ./src/client.cpp
)
# CURL 찾기
find_package(CURL REQUIRED)

# JSON 헤더 라이브러리 찾기
find_package(nlohmann_json QUIET)

if(nlohmann_json_FOUND)
    target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/json)
endif()

# CURL 라이브러리 연결
target_link_libraries(server PRIVATE CURL::libcurl)
