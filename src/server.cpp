#include "server.h"
#include <thread>
#include <chrono>

Server::Server() {
    osrm = std::make_unique<OSRM>();
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        return;
    }

    std::cout << "[서버] 연결 대기 중...\n";
    client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    std::cout << "[서버] 클라이언트 연결됨\n";
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
}

Server::~Server() {
    if (client_fd >= 0) close(client_fd);
    if (server_fd >= 0) close(server_fd);
    unlink(SOCKET_PATH);
}

float Server::getCo2() {
    // Implementation for getting CO2 data
    return 0.0f; // Placeholder return value
}

float Server::getSleep() {
    // Implementation for getting sleep data
    return 0.0f; // Placeholder return value
}

void Server::getGps() {
    // Implementation for getting GPS data
    last_gps = cur_gps; // Update last_gps with current gps
}




double Server::calculateBearing(double lat1, double lon1, double lat2, double lon2) {
    lat1 *= DEG_TO_RAD;
    lat2 *= DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) -
               sin(lat1) * cos(lat2) * cos(dLon);

    double bearingRad = atan2(y, x);
    double bearingDeg = fmod((RAD_TO_DEG * bearingRad + 360.0), 360.0);

    return bearingDeg;
}


ClientData Server::receiveClient() {
    ClientData data;
    ssize_t len = recv(client_fd, &data, sizeof(data), 0);
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return data; // No data received
        perror("recv");
        return data; // Return empty data on error
    }
    return data;
}

int Server::SendClient(const ServerData& data) {
    ssize_t len = send(client_fd, &data, sizeof(data), 0);
    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return -1;
        perror("send");
        return -1;
    }
    return 0;
}

int Server::run() {
    getGps();
    ClientData client_data = receiveClient();
    if (client_data.is_navigating) {
        is_navigating = true;
    }
    // 단위 m/s
    float speed = osrm->haversine(last_gps.first, last_gps.second, cur_gps.first, cur_gps.second) / REPEAT_TIME;
    // 경로 탐색 알고리즘
    ServerData server_data = {
        getCo2(),
        getSleep(),
        speed, 
        {}, // 턴 정보 초기화
        0 // 턴 정보 개수 초기화
    };
    if (is_navigating) {
        double bearing = calculateBearing(last_gps.first, last_gps.second, cur_gps.first, cur_gps.second);
        //bearing = 0.0;
        //cur_gps = {37.5665, 126.9780}; // 예시 GPS 좌표, 실제 GPS 데이터로 대체 필요
        osrm->init(cur_gps.second, cur_gps.first, bearing);
        // 초당 몇번 할건지? 
        osrm->getTurnInfo(cur_gps.second, cur_gps.first, bearing, server_data.turnInfos, server_data.turnInfoCount);
        if (server_data.turnInfoCount == 1 && server_data.turnInfos[0].distance < 100) {
            is_navigating = false; // 목적지 도착
        }
        else if (server_data.turnInfoCount < 0) {
            is_navigating = false; // 목적지 도착
        }
    }
    
    SendClient(server_data);
    return 0;
}

int main(void) {
    std::unique_ptr<Server> server = std::make_unique<Server>();
    while (!server->run()) {
        // sleep until REPEAT_TIME
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(REPEAT_TIME)));
    }
    return 0;
}