#include "osrm.h"


void OSRM::readDataFromCSV(const std::string& filename, std::vector<RestArea>& areas)  {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "파일을 열 수 없습니다: " << filename << "\n";
        return;
    }
    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (isHeader) {
            isHeader = false;
            continue; // 헤더 스킵
        }

        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> tokens;

        while (std::getline(ss, cell, ',')) {
            tokens.push_back(cell);
        }

        if (tokens.size() >= 11) {
            RestArea area;
            area.name = tokens[0];

            try {
                area.lat = std::stod(tokens[9]);
                area.lon = std::stod(tokens[10]);
                areas.push_back(area);
            } catch (...) {
                // 숫자 변환 실패 시 스킵
                continue;
            }
        }
    }
    file.close();
}
// restAreas에 CSV 파일에서 데이터를 읽어 초기화
OSRM::OSRM(){
    std::vector<RestArea> restAreas;
    readDataFromCSV(REST_CSV_FILE, restAreas);
    readDataFromCSV(REST_CSV_FILE2, restAreas); // 휴게소 데이터를 읽어옵니다.
    root = buildKDTree(restAreas);
    restAreas.clear(); // 메모리 절약을 위해 restAreas 벡터를 비웁니다.
}

OSRM::~OSRM() {
    destroyKDTree(root);
}

// lon과 lat을 초기화하고, restAreas에서 가장 가까운 rest 정보를 찾는 함수
void OSRM::init(double lon, double lat, double brng) {
    double mindist = std::numeric_limits<double>::max();
    this->lon = lon;
    this->lat = lat;
    std::priority_queue<std::pair<double, RestArea>> maxHeap;
    kNearestSearch(root, lat, lon, 0, maxHeap, kRestSearchCount);
    while (!maxHeap.empty()) {
        double dist = distance(lat, lon, maxHeap.top().second.lat, maxHeap.top().second.lon, brng);
        if (dist < 0) {
            std::cerr << "거리 계산 오류: 음수 거리\n";
            maxHeap.pop();
            continue;
        }
        //std::cout << "휴게소: " << maxHeap.top().second.name
        //     << ", 거리: " << dist << "m\n" << std::endl;
        if (dist < mindist) {
            mindist = dist;
            rest = maxHeap.top().second;
        }
        maxHeap.pop();
    }
    std::cout << "가장 가까운 휴게소: " << rest.name << " (" << rest.lon << ", " << rest.lat << ")\n";
}

// 두 지점 사이의 경로 거리 계산 함수
double OSRM::distance(double lat1, double lon1, double lat2, double lon2, double brng) {
    std::ostringstream url;
    url << OSRM_URL << "route/v1/driving/"
        << lon1 << "," << lat1 << ";" << lon2 << "," << lat2
        << "?overview=false" << "&bearings=" << static_cast<int>(brng) << BEARING_RANGE;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL 초기화 실패\n";
        return -1.0;
    }
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL 요청 실패: " << curl_easy_strerror(res) << "\n";
        return -1.0;
    }
    try {
        json j = json::parse(response);
        return j["routes"][0]["distance"];
    } catch (const std::exception& e) {
        std::cerr << "JSON 파싱 오류: " << e.what() << "\n";
    }
    return -1.0;
}

// 위치 정보와 방향을 기반으로 경로 정보를 불러옴
void OSRM::getTurnInfo(double lon, double lat , double brng, TurnInfo* turninfos, size_t &i) {
    std::ostringstream url;
    url << OSRM_URL << "route/v1/driving/"
        << lon << "," << lat << ";" << rest.lon << "," << rest.lat
        << "?steps=true" << "&bearings=" << static_cast<int>(brng) << BEARING_RANGE;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL 초기화 실패\n";
        return;
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL 요청 실패: " << curl_easy_strerror(res) << "\n";
        return;
    }

    try {
        json j = json::parse(response);
        auto steps = j["routes"][0]["legs"][0]["steps"];
        double totalDistance = j["routes"][0]["distance"];

        std::cout << "총 거리: " << totalDistance << "m\n";
        std::cout << "턴 정보:\n";
        for (const auto& step : steps) {
            std::string road = step["name"];
            std::string modifier = step["maneuver"].value("modifier", "직진");
            std::string type = step["maneuver"]["type"];
            double distance = step["distance"];

            if (type == "arrive") continue;

            //if (type == "depart" || type == "arrive") continue;
            if (i >= 4) {
                break; // 최대 4개의 턴 정보만 저장
            }
            turninfos[i] = TurnInfo{
                "",
                (modifier == "left") ? modifier::Left :
                (modifier == "right") ? modifier::Right :
                (modifier == "slight left") ? modifier::Slight_left :
                (modifier == "slight right") ? modifier::Slight_right :
                (modifier == "uturn") ? modifier::UTurn : modifier::Straight,
                distance
            };
            strncpy(turninfos[i].road, road.c_str(), sizeof(turninfos[i].road) - 1);
            turninfos[i].road[sizeof(turninfos[i].road) - 1] = '\0'; // 문자열 종료 보장
            turninfos[i].mod = (modifier == "left") ? modifier::Left :
                (modifier == "right") ? modifier::Right :
                (modifier == "slight left") ? modifier::Slight_left :
                (modifier == "slight right") ? modifier::Slight_right :
                (modifier == "uturn") ? modifier::UTurn : modifier::Straight;
            turninfos[i].distance = distance;
            i++;
            std::cout << "  - " << distance << "m 앞에서 " << road << " 방향으로 " << modifier << " 회전\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON 파싱 오류: " << e.what() << "\n";
    }
}

// KD-트리를 구축하는 함수
KDNode *OSRM::buildKDTree(std::vector<RestArea> &areas, int depth) {
        if (areas.empty()) return nullptr;

    size_t axis = depth % 2;  // 0: 경도(x), 1: 위도(y)
    size_t mid = areas.size() / 2;

    std::nth_element(areas.begin(), areas.begin() + mid, areas.end(), [&](const RestArea& a, const RestArea& b) {
        return (axis == 0) ? a.lon < b.lon : a.lat < b.lat;
    });

    std::vector<RestArea> left(areas.begin(), areas.begin() + mid);
    std::vector<RestArea> right(areas.begin() + mid + 1, areas.end());

    KDNode* node = new KDNode{areas[mid]};
    node->left = buildKDTree(left, depth + 1);
    node->right = buildKDTree(right, depth + 1);

    return node;
}


void OSRM::kNearestSearch(KDNode *node, double targetLat, double targetLon, int depth, std::priority_queue<std::pair<double, RestArea>> &maxHeap, int k) {
    if (!node) return;

    double d = haversine(targetLat, targetLon, node->area.lat, node->area.lon);

    // 힙이 가득 차면, 가장 먼 요소보다 가까울 때만 교체
    if ((int)maxHeap.size() < k) {
        maxHeap.push({d, node->area});
    } else if (d < maxHeap.top().first) {
        maxHeap.pop();
        maxHeap.push({d, node->area});
    }

    int axis = depth % 2;
    double diff = (axis == 0)
                    ? targetLon - node->area.lon
                    : targetLat - node->area.lat;

    KDNode* first = (diff < 0) ? node->left : node->right;
    KDNode* second = (diff < 0) ? node->right : node->left;

    kNearestSearch(first, targetLat, targetLon, depth + 1, maxHeap, k);

    // 반대편 서브트리도 확인할 조건
    if (maxHeap.size() < (size_t)k || std::abs(diff) < maxHeap.top().first) {
        kNearestSearch(second, targetLat, targetLon, depth + 1, maxHeap, k);
    }
}

// Haversine 공식을 사용하여 두 지점 사이의 거리를 계산하는 함수
double OSRM::haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1) * cos(lat2) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return kEarthRadius * c * 1000; // m
}

void OSRM::destroyKDTree(KDNode *node) {
    if (!node) return;
    destroyKDTree(node->left);
    destroyKDTree(node->right);
    delete node;
}
