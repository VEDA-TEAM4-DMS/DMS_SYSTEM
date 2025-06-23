cmake_minimum_required(VERSION 3.14)
project(MyOSRMApp)

set(CMAKE_CXX_STANDARD 17)

# Boost, TBB, GDAL 등 외부 의존성
find_package(Boost REQUIRED COMPONENTS system filesystem thread regex iostreams program_options)
find_package(TBB REQUIRED)

# Include 디렉토리
include_directories(
    src
    thirdparty/osrm-backend/include
)

# OSRM을 서브디렉토리로 추가 (외부 레포지토리 cmake 그대로 사용)
add_subdirectory(thirdparty/osrm-backend EXCLUDE_FROM_ALL)

# 내 osrm.cpp 실행 파일
add_executable(osrm_app
    src/osrm.cpp
)

# OSRM이 만든 타겟을 링크
target_link_libraries(osrm_app
    osrm           # 메인 OSRM 인터페이스
    osrm_engine    # 엔진 구현
    osrm_util      # 유틸리티
    ${Boost_LIBRARIES}
    TBB::tbb
    pthread
    bz2
    z
    gdal
)
    