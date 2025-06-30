#include "client.h"

Client::Client()
    : speakerThread(&Client::SpeakerThread, this)
{
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    connect(sock, (sockaddr*)&addr, sizeof(addr));
    std::cout << "[클라이언트] 연결됨\n";

    struct sockaddr_rc raddr = { 0 };
    blue_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    raddr.rc_family = AF_BLUETOOTH;
    raddr.rc_channel = (uint8_t) 1; // 채널 번호 (장치에 따라 다름)
    str2ba(BT_ADDR, &raddr.rc_bdaddr);
    if (connect(blue_sock, (struct sockaddr *)&raddr, sizeof(raddr)) == 0) {
        std::cout << "[클라이언트] 블루투스 연결됨\n";
    }
    else {
        std::cerr << "[클라이언트] 블루투스 연결 실패\n";
    }
    // tts init
    tts = std::make_unique<TTS>();
}

Client::~Client() {
    close(sock);
    close(blue_sock);
}

void Client::LED() {
    // Implementation for LED notification
    // This could be a placeholder for actual LED control code
    std::cout << "[클라이언트] LED 알림\n";
}


void Client::Speaker() {
    auto speaker = notice_queue.front();
    notice_queue_mutex.unlock();
    float time = 100.0f; // ms 단위
    switch (speaker.first) {
        case NOTICE_SPEAKER::Co2:
            // Implementation for CO2 notification
            tts->run("CO2 concentration is high. Please ventilate.");
            break;
        case NOTICE_SPEAKER::Navigate:
            // Implementation for navigation notification
            tts->run("Should I navigate you to the nearest rest area?");
            break;
        case NOTICE_SPEAKER::Navigating:
            tts->run(speaker.second);
            break;
        default:
            break;
    }
    std::unique_lock<std::mutex> lock(speaker_mutex);
    speaker_cv.wait_for(lock, std::chrono::milliseconds(static_cast<int>(time)), [this] {
        return speaker_active;
    });
    if (speaker_active) {
        lock.unlock();
        // case NOTICE_SPEAKER::Sleep:
        tts->run("wake please get up");
        lock.lock();
        speaker_active = false;
        lock.unlock();
        // 일단 sleep 경고 이후 이전에 실행하던 알림 실행
        return;
    }
    lock.unlock();
    notice_queue_mutex.lock();
    notice_queue.pop();
    notice_queue_mutex.unlock();

    notice_set_mutex.lock();
    notice_set.erase(speaker);
    notice_set_mutex.unlock();

    last_notice = speaker.first;

}

void Client::SpeakerThread() {
    while (true) {
        notice_queue_mutex.lock();
        if (!notice_queue.empty()) {
            Speaker();
        }
        else {
            notice_queue_mutex.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust sleep time as needed
    }
}

void Client::SpeakerPush(NOTICE_SPEAKER speaker) {
    // Implementation for pushing speaker notifications
    std::string str = "";
    if (speaker == NOTICE_SPEAKER::Sleep) {
        speaker_mutex.lock();
        speaker_active = true;
        speaker_mutex.unlock();
    }
    else {
        notice_set_mutex.lock();
        if (notice_set.find({speaker, str}) == notice_set.end()) {
            notice_set.insert({speaker, str});
            notice_set_mutex.unlock();
            notice_queue_mutex.lock();
            notice_queue.push({speaker, str});
            notice_queue_mutex.unlock();
        } else {
            notice_set_mutex.unlock();
        }
    }
}

void Client::NavPush(TurnInfo &turn) {
    NOTICE_SPEAKER speaker = NOTICE_SPEAKER::Navigating;
    std::string str = "It turns ";
    switch (turn.mod) {
        case modifier::Right : 
            str += "right ";
            break;
        case modifier::Left :
            str += "left ";
            break;
        default : 
            str += "straight ";
            break;
    }
    str += "to " + std::string(turn.road) + " road in front of " + std::to_string((int)turn.distance)
        + " meters in front of you.";
    notice_set_mutex.lock();
    if (notice_set.find({speaker, str}) == notice_set.end()) {
        notice_set.insert({speaker, str});
        notice_set_mutex.unlock();
        notice_queue_mutex.lock();
        notice_queue.push({speaker, str});
        notice_queue_mutex.unlock();
    } else {
        notice_set_mutex.unlock();
    }
}

GESTURE Client::getGesture()
{
    return GESTURE::Default;
}

ServerData Client::run() {
    ServerData data{};
    if (recv(sock, &data, sizeof(data), 0) <= 0) {
        data.co2 = -1.0f;
        return data;
    }
    if (data.co2 > CO2_THRESHOLD) {
        SpeakerPush(NOTICE_SPEAKER::Co2);
    }
    WARN cond = WARN::Normal;
    if (data.sleep > SLEEP_DANGER_THRESHOLD) {
        cond = WARN::Danger;
        SpeakerPush(NOTICE_SPEAKER::Sleep);
        LED();
        if (!is_navigating) {
            SpeakerPush(NOTICE_SPEAKER::Navigate);
        }
    }
    else if (data.sleep > SLEEP_WARN_THRESHOLD) {
        cond = WARN::Warning;
        if (!is_navigating) {
            SpeakerPush(NOTICE_SPEAKER::Navigate);
        }
    }
    sendBT(cond);
    GESTURE gesture = getGesture();
    if (last_gesture != gesture && last_notice == NOTICE_SPEAKER::Navigate) {
        if (gesture == GESTURE::Accept) {
            is_navigating = true;
            sendFunc({true});
            last_notice = NOTICE_SPEAKER::Default;
        } else if (gesture == GESTURE::Reject) {
            is_navigating = false;
            last_notice = NOTICE_SPEAKER::Default;
        }
    }
    last_gesture = gesture;
    if (data.turnInfoCount > 0) {
        int cur_h = data.turnInfos[0].distance / 100;
        int last_h = last_distance / 100;
        if (cur_h != last_h) {
            // 몇 미터 앞에서 회전하는지에 대한 알림
            NavPush(data.turnInfos[0]);
        }
        last_distance = data.turnInfos[0].distance;
        /*for (int i=0;i<data.turnInfoCount; i++) {
            std::cout << "도로: " << data.turnInfos[i].road << ", 방향: " << static_cast<int>(data.turnInfos[i].mod)
                      << ", 거리: " << data.turnInfos[i].distance << "m\n";
        }*/
    }
    return data;
}


int Client::sendFunc(const ClientData &data)
{
    send(sock, &data, sizeof(data), 0);
    return 0; // Placeholder return value
}

int Client::sendBT(const WARN &a) {
    send(blue_sock, &a, sizeof(a), 0);
    return 0;
}
