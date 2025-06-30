#ifndef GPS_READER_H
#define GPS_READER_H
#include <string>
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <chrono> // 시간 측정

/**
 * 위도/경도 좌표 구조체
 */
struct GPSCoordinate {
    double latitude;
    double longitude;
};

/**
 * GPSReader 클래스
 * - getCoordinateFromSerial(): UART 포트에서 위경도 읽어오기
 */
class GPSReader {
public:
    static bool configureSerialPort(int fd, speed_t baudrate);
    /**
     * 메인 API
     * - 시리얼 장치 경로
     * - 위도/경도 좌표 결과
     * - 타임아웃 (기본 10초)
     * - 성공(true)/실패(false) 리턴
     */
    static bool readRawSentence(std::string& outSentence, int timeout_sec = 10);
    static bool parseNMEASentence(const std::string& sentence, GPSCoordinate& coord);
    static bool getCoordinateFromSerial(GPSCoordinate& coord, int timeout_sec);
};
#endif