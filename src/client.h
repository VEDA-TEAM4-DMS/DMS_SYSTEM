#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <memory>
#include "common.h"

#define CO2_THRESHOLD 0.5f
#define SLEEP_THRESHOLD 0.5f
#define SPEED_THRESHOLD 0.5f

enum class GESTURE {
    Default,
    Accept,
    Reject,
    Quit
};

enum class NOTICE_SPEAKER {
    Default,
    Co2,
    Sleep,
    Navigate
};

class Client {
public:
    Client();
    void LED();
    int run();
    void SpeakerThread();
private:
    ServerData receiveFunc();
    int sendFunc(const ClientData& data);
    void SpeakerPush(NOTICE_SPEAKER speaker);
    GESTURE getGesture();
    void Speaker();

    bool is_navigating = false;
    NOTICE_SPEAKER last_notice = NOTICE_SPEAKER::Default;
    GESTURE last_gesture = GESTURE::Default;
    std::queue<NOTICE_SPEAKER> notice_queue;
    std::mutex notice_queue_mutex;
    std::unordered_set<NOTICE_SPEAKER> notice_set;
    std::mutex notice_set_mutex;

    float last_distance = 0.0f;
    int sock;
};