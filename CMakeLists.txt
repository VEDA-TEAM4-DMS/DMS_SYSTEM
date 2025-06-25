cmake_minimum_required(VERSION 3.10)
project(OSRMClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 실행 파일 이름과 소스
add_executable(osrm ./src/osrm.cpp)

# 필요 라이브러리 추가
find_package(CURL REQUIRED)

# nlohmann/json은 보통 헤더 전용으로 설치되어 있어도 find_package로 검색 가능
# 실패 시 수동 include 디렉토리 지정 필요
find_package(nlohmann_json QUIET)

if(nlohmann_json_FOUND)
    target_link_libraries(osrm PRIVATE nlohmann_json::nlohmann_json)
else()
    # 수동 include
    target_include_directories(osrm PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/json)
endif()

# CURL 라이브러리 연결
target_link_libraries(osrm PRIVATE CURL::libcurl)
