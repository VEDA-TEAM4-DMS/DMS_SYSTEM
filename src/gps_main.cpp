#include <iostream>
#include "GPSReader.h"

int main(){
    std::string sentence;
    struct GPSCoordinate gpscoor;

    if(GPSReader::getCoordinateFromSerial(gpscoor, 1)){
        //std::cout << "receviced raw NMEA: " << sentence << std::endl;
        std::cout << "latitude: " << gpscoor.latitude << " longitude: " << gpscoor.longitude << std::endl;
    }
    else{
        std::cout << "Timeout or read error!" << std::endl;
    }

}