cmake_minimum_required(VERSION 3.18)
project(DMS_OSRM_System LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# OSRM 백엔드 소스 포함 (Git 서브모듈 / 복사 경로 기준)
add_subdirectory(thirdparty/osrm-backend)

# 시스템 종속성
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options thread iostreams regex date_time)

# Include 디렉토리 추가 (CURL 포함)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/osrm-backend/include
    ${CURL_INCLUDE_DIRS}
)

add_executable(osrm_custom_app
    src/osrm.cpp
)

# OSRM + 기타 라이브러리 링크
target_link_libraries(osrm_custom_app
    osrm                  # libosrm
    CURL::libcurl         # curl
    Threads::Threads      # pthread
    Boost::program_options
    Boost::thread
    Boost::iostreams
    Boost::regex
    Boost::date_time
)

# 추가 include 및 compile 옵션
target_include_directories(osrm_custom_app PRIVATE
    ${thirdparty_osrm-backend_SOURCE_DIR}/include
)

# 필요시 -DOSRM_PROJECT_DIR와 같은 define 사용
target_compile_definitions(osrm_custom_app PRIVATE
    -DOSRM_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/osrm-backend"
)
