#include "../common.h"
#include "tts.h"

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
#include <condition_variable>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define CO2_THRESHOLD 0.5f
#define SLEEP_DANGER_THRESHOLD 0.5f
#define SLEEP_WARN_THRESHOLD 0.3f
#define SPEED_THRESHOLD 0.5f

#define BT_ADDR "00:1A:7D:DA:71:13"

enum class WARN {
    Normal,
    Warning,
    Danger
};

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
    Navigate,
    Navigating
};

namespace std {
    template <>
    struct hash<std::pair<NOTICE_SPEAKER, std::string>> {
        size_t operator()(const std::pair<NOTICE_SPEAKER, std::string>& p) const {
            size_t h1 = hash<int>()(static_cast<int>(p.first));
            size_t h2 = hash<std::string>()(p.second);
            return h1 ^ (h2 << 1); // 해시 조합
        }
    };
}

class Client {
public:
    Client();
    ~Client();
    void LED();
    ServerData run();
    void SpeakerThread();


private:
    int sendFunc(const ClientData& data);
    void SpeakerPush(NOTICE_SPEAKER speaker);
    void NavPush(TurnInfo& turn);
    GESTURE getGesture();
    void Speaker();
    int sendBT(const WARN& i);

    bool is_navigating = false;
    NOTICE_SPEAKER last_notice = NOTICE_SPEAKER::Default;
    GESTURE last_gesture = GESTURE::Default;
    std::queue<std::pair<NOTICE_SPEAKER, std::string>> notice_queue;
    std::mutex notice_queue_mutex;
    std::unordered_set<std::pair<NOTICE_SPEAKER, std::string>> notice_set;
    std::mutex notice_set_mutex;

    float last_distance = 0.0f;
    int sock;
    int blue_sock;
    std::condition_variable speaker_cv;
    std::mutex speaker_mutex;
    bool speaker_active = false;

    std::thread speakerThread;

    std::unique_ptr<TTS> tts;
};