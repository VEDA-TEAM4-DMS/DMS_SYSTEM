#include "GPSReader.h"

std::string device = "/dev/ttyS0";

// serial 통신 설정
bool GPSReader::configureSerialPort(int fd, speed_t baudrate)
{   struct termios options;
    if(tcgetattr(fd, &options) < 0){
        perror("tcgetattr");
        return false;
    }

    // 속도 설정
    cfsetispeed(&options, baudrate); 
    cfsetospeed(&options, baudrate);

    // 8N1
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB; // no parity
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // RAW 모드
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 문자(스트림) 단위 즉시 읽기
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST; // 출력 후처리(\n -> \n\r) 해제

    if(tcsetattr(fd, TCSANOW, &options) < 0){
        perror("tcsetattr");
        return false;
    }
    return true;

}

// 한 문장 읽기 API 
bool GPSReader::readRawSentence(std::string& outSentence, int timeout_sec){
    // 1.  시리얼 장치 열기
    
    int fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return false;
    }

    // 2. serial 통신 설정
    if (!configureSerialPort(fd, B9600)){
        close(fd);
        return false;
    }

    // 3. select() 준비
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    std::string buffer;
    char c;
    bool lineComplete = false;

    // 4. 읽기 루프
    while (true) {
        fd_set tempfds = readfds;
        // 데이터 올 때까지 대기(현재는 5초로 timeout 설정)
        int ret = select(fd + 1, &tempfds, nullptr, nullptr, &timeout);
        if (ret < 0) {
            perror("select");
            close(fd);
            return false;
        } else if (ret == 0) {
            std::cout << "[GPSReader] Timeout: no data within " << timeout_sec << " seconds" << std::endl;
            close(fd);
            return false;
        } else {
            // 읽기
            int n = read(fd, &c, 1);
            if (n > 0) {
                if (c == '\n' || c == '\r') {
                    if (!buffer.empty()) {
                        lineComplete = true;
                        break;
                    }
                } else {
                    buffer += c;
                }
            }
        }
    }

    close(fd);

    if (lineComplete) {
        outSentence = buffer;
        std::cout << "receviced raw NMEA: " << outSentence << std::endl;
        return true;
    }

    return false;

}

// 각 필드값 저장하는 함수
static void splitCSV(const std::string& line, std::vector<std::string>& fields)
{
    std::stringstream ss(line);
    std::string token;

    while(std::getline(ss, token, ',')){
        fields.push_back(token);
    }
}

static double convertToDecimal(const std::string& raw, char direction) {
    double val = std::atof(raw.c_str());
    double degrees = std::floor(val / 100);
    double minutes = val - (degrees * 100);
    double decimal = degrees + (minutes / 60.0);
    if(direction == 'S' || direction == 'W') decimal = -decimal;
    return decimal;

}

// parsing 함수
bool GPSReader::parseNMEASentence(const std::string& sentence, GPSCoordinate& coord)
{
    // 지원 문장인지 검사
    if (sentence.find("$GPRMC") != 0 && sentence.find("$GPGGA") != 0 &&
        sentence.find("$GNRMC") != 0 && sentence.find("$GNGGA") != 0) {
        return false;
    }

    std::vector<std::string> fields;
    splitCSV(sentence, fields);

    int latIdx, latDirIdx, lonIdx, lonDirIdx;
    if(fields[0].find("GGA") != std::string::npos){
        latIdx = 2;
        latDirIdx = 3;
        lonIdx = 4;
        lonDirIdx = 5;
    }
    else if(fields[0].find("RMC") != std::string::npos){
        latIdx = 3;
        latDirIdx = 4;
        lonIdx = 5;
        lonDirIdx = 6;
    }
    else{
        return false;
    }

    // 필드 개수가 우리가 필요로 하는 인덱스보다 적으면 실패
    if(fields.size() <= std::max(std::max(latIdx, latDirIdx),
                       std::max(lonIdx, lonDirIdx)))
    {
        return false;
    }

    coord.latitude = convertToDecimal(fields[latIdx], fields[latDirIdx][0]);
    coord.longitude = convertToDecimal(fields[lonIdx], fields[lonDirIdx][0]);

    return true;
}

// 위도 경도 얻는 API
bool GPSReader::getCoordinateFromSerial(GPSCoordinate& coord, int timeout_sec) 
{
    auto start_time = std::chrono::steady_clock::now();

    while(true)
    {
        // 남은 시간 계산
        auto now = std::chrono::steady_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        if(elapsed >= timeout_sec){
            std::cerr << "[GPS Reader] Timeout : no valid sentence in " << timeout_sec << "seconds" << std::endl;
            return false;
        }

        int remaining = timeout_sec - elapsed;
        std::cout << "reaming time: " << remaining << std::endl;

        std::string sentence;
        if(!readRawSentence(sentence, remaining)){
            continue;
        } // 문장 제대로 안읽혔으면 다시 읽기

        // GPRMC / GPGGA 문장인지 확인
        if(sentence.find("$GPRMC") != 0 && sentence.find("$GPGGA") != 0 &&
           sentence.find("$GNRMC") != 0 && sentence.find("$GNGGA") != 0){
            std::cout << "[GPS] no need sentence !!! " << std::endl;
            continue; // 필요 없는 문장으로 다시 읽기
        }

        if(parseNMEASentence(sentence, coord)){
            return true;
        }
    }
}