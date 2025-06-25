#include <iostream>
#include <memory>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>
#include <queue>
#include <curl/curl.h>
#include "json.hpp"  // nlohmann::json 

#define OSRM_URL "http://localhost:5000/"
#define DATA_PATH "../res"
#define REST_CSV_FILE DATA_PATH "/rest.csv"
#define REST_CSV_FILE2 DATA_PATH "/rest_area.csv"
#define kEarthRadius 6371.0
#define kRestSearchCount 10
#define BEARING_RANGE ",30;" // 방향 허용 범위

// 휴게소 정보를 저장하는 구조체
struct RestArea {
    std::string name;
    double lat;
    double lon;
    bool operator< (const RestArea& other) const {
        return (lat < other.lat) || (lat == other.lat && lon < other.lon);
    }
};

// KD-트리 노드를 정의하는 구조체
struct KDNode {
    RestArea area;
    KDNode* left = nullptr;
    KDNode* right = nullptr;
};


class OSRM {
    using json = nlohmann::json;
public:
    OSRM();
    ~OSRM();
    void init(double lon, double lat, double brng);
    double distance(double lat1, double lon1, double lat2, double lon2, double brng);
    void getTurnInfo(double lon, double lat, double brng);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t totalSize = size * nmemb;
        output->append((char*)contents, totalSize);
        return totalSize;
    };
    void readDataFromCSV(const std::string& filename, std::vector<RestArea>& areas);
    KDNode* buildKDTree(std::vector<RestArea>& areas, int depth = 0);
    void kNearestSearch(KDNode* node, double targetLat, double targetLon, int depth,
                    std::priority_queue<std::pair<double, RestArea>>& maxHeap, int k);
    // 두 지점 사이의 유클리드 거리 계산
    double haversine(double lat1, double lon1, double lat2, double lon2);
    void destroyKDTree(KDNode* node);
    double lon;
    double lat;
    RestArea rest;
    KDNode* root = nullptr;  // KD-트리의 루트 노드
};

