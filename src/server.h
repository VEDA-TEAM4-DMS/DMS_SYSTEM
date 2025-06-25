#include <unordered_set>
#include <mutex>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include "osrm.h"
#include "common.h"


#define REPEAT_TIME 1000.0
#define CO2_FILE ""
#define GPS_FILE ""
#define CO2_THRESHOLD 0.5f
#define SLEEP_THRESHOLD 0.5f
#define SPEED_THRESHOLD 0.5f

constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / M_PI;



class Server {
public:
    Server();
    ~Server();
    int run();
private:
    float getCo2();
    float getSleep();
    void getGps();


    double calculateBearing(double lat1, double lon1, double lat2, double lon2);

    ClientData receiveClient();

    int SendClient(const ServerData &data);


    std::unique_ptr<OSRM> osrm;
    std::pair<double, double> last_gps = {0.0f, 0.0f};
    std::pair<double, double> cur_gps = {0.0f, 0.0f};
    bool is_navigating = false;
    std::vector<TurnInfo> turnInfos;

    int server_fd = -1;
    int client_fd = -1;
};
