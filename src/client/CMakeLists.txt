cmake_minimum_required(VERSION 3.5)
project(client LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES untitled_ko_KR.ts)

set(PROJECT_SOURCES
    qtui.cpp
    client.cpp
    widget.cpp
    widget.h
    widget.ui
    tts.cpp         # ✅ 추가
    tts.h           # ✅ 추가
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    add_executable(client ${PROJECT_SOURCES})
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
endif()

# 공통 헤더 포함
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/src/common)

# Qt 라이브러리 링크
target_link_libraries(client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# piper 및 관련 의존 라이브러리 경로
target_include_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/src/cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/fi/include
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/si/include
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/pi/include
    /usr/include/onnxruntime
)

target_link_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/fi/lib
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/si/lib
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/pi/lib
)

target_link_libraries(client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${BLUETOOTH_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/thirdparty/piper/build/libpiper.a
    -lespeak-ng
    -lonnxruntime
    -lfmt
    -lspdlog
    -lpiper_phonemize
    -lpthread
)


# 블루투스
find_package(PkgConfig REQUIRED)
pkg_check_modules(BLUETOOTH REQUIRED bluez)
target_link_libraries(client PRIVATE ${BLUETOOTH_LIBRARIES})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(client)
endif()

# client 타겟 생성 이후 아래 추가
set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)