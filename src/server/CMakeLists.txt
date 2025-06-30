cmake_minimum_required(VERSION 3.10)
project(server LANGUAGES CXX)

add_executable(server
    server.cpp
    osrm.cpp
)

# 공통 헤더 포함
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src/common)

# nlohmann json (header-only)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # json.hpp 포함 경로
endif()

# CURL
find_package(CURL REQUIRED)
target_link_libraries(server PRIVATE CURL::libcurl)

# 블루투스
find_package(PkgConfig REQUIRED)
pkg_check_modules(BLUETOOTH REQUIRED bluez)
target_link_libraries(server PRIVATE ${BLUETOOTH_LIBRARIES})

# server 타겟 생성 이후 아래 추가
set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)