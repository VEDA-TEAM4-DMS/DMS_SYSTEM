#pragma once
#include <vector>
#include <string>

#define SOCKET_PATH "/tmp/ipc_struct.sock"

enum class modifier {
    Straight,
    Left,
    Right,
    UTurn,
    Slight_left,
    Slight_right
};

struct TurnInfo {
    char road[128];
    modifier mod;
    double distance;
};

struct ServerData {
    float co2;
    float sleep;
    float speed;
    TurnInfo turnInfos[4]; // 최대 4개의 턴 정보
    size_t turnInfoCount = 0; // 실제 턴 정보 개수
};

struct ClientData {
    bool is_navigating = false;
};