#include "client.h"

Client::Client()
{
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    connect(sock, (sockaddr*)&addr, sizeof(addr));
    std::cout << "[클라이언트] 연결됨\n";
}

void Client::LED() {
    // Implementation for LED notification
    // This could be a placeholder for actual LED control code
    std::cout << "[클라이언트] LED 알림\n";
}

void Client::Speaker() {
    NOTICE_SPEAKER speaker = notice_queue.front();
    notice_queue_mutex.unlock();
    switch (speaker) {
        case NOTICE_SPEAKER::Co2:
            // Implementation for CO2 notification
            break;
        case NOTICE_SPEAKER::Sleep:
            // Implementation for sleep notification
            break;
        case NOTICE_SPEAKER::Navigate:
            // Implementation for navigation notification
            break;
        default:
            break;
    }
    notice_queue_mutex.lock();
    notice_queue.pop();
    notice_queue_mutex.unlock();
    notice_set_mutex.lock();
    notice_set.erase(speaker);
    notice_set_mutex.unlock();
    last_notice = speaker;
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
    notice_set_mutex.lock();
    if (notice_set.find(speaker) == notice_set.end()) {
        notice_set.insert(speaker);
        notice_set_mutex.unlock();
        notice_queue_mutex.lock();
        notice_queue.push(speaker);
        notice_queue_mutex.unlock();
    } else {
        notice_set_mutex.unlock();
    }
    last_notice = speaker;
}

GESTURE Client::getGesture()
{
    return GESTURE::Default;
}

int Client::run() {
    ServerData data = receiveFunc();
    if (data.co2 > CO2_THRESHOLD) {
        SpeakerPush(NOTICE_SPEAKER::Co2);
    }
    if (data.sleep > SLEEP_THRESHOLD) {
        SpeakerPush(NOTICE_SPEAKER::Sleep);
        LED();
        if (!is_navigating) {
            SpeakerPush(NOTICE_SPEAKER::Navigate);
        }
    }
    GESTURE gesture = getGesture();
    if (last_gesture != gesture && last_notice == NOTICE_SPEAKER::Navigate) {
        if (gesture == GESTURE::Accept) {
            is_navigating = true;
            sendFunc({true});
        } else if (gesture == GESTURE::Reject) {
            is_navigating = false;
        }
    }
    last_gesture = gesture;
    if (data.turnInfoCount > 0) {
        int cur_h = data.turnInfos[0].distance / 100;
        int last_h = last_distance / 100;
        if (cur_h != last_h) {
            // 몇 미터 앞에서 회전하는지에 대한 알림
            SpeakerPush(NOTICE_SPEAKER::Navigate);
        }
        last_distance = data.turnInfos[0].distance; 
        for (int i=0;i<data.turnInfoCount; i++) {
            std::cout << "도로: " << data.turnInfos[i].road << ", 방향: " << static_cast<int>(data.turnInfos[i].mod) 
                      << ", 거리: " << data.turnInfos[i].distance << "m\n";
        }
    }
    return 0;
}

ServerData Client::receiveFunc() {
    ServerData data{};
    recv(sock, &data, sizeof(data), 0);
    return data;
}

int Client::sendFunc(const ClientData &data)
{
    send(sock, &data, sizeof(data), 0);
    return 0; // Placeholder return value
}

int main() {
    std::unique_ptr<Client> client = std::make_unique<Client>();
    std::thread speakerThread(&Client::SpeakerThread, client.get());
    while(!client->run()) {
        // Main loop for client operations
        //std::cout << "[클라이언트] 클라이언트 실행 중...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust sleep time as needed
    }
    return 0;
}
