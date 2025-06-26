#include <stdio.h>
#include <unistd.h> // close, usleep 등 시스템 호출
#include <fcntl.h> // open 함수
#include <sys/ioctl.h> // ioctl 함수
#include <stdlib.h> // exit 함수

#include "dms_led.h"

int main()
{
    int flag = 0;
    int fd ;

    // /dev/rpihat 디바이스 파일 열기
    fd = open("/dev/dms_led", O_RDWR);
    printf("fd = %d\n", fd);

    if(fd < 0){
        perror("/dev/dms_led open error !!!");
        printf("[led_app] please check if /dev/dms_led exist !!!");
        exit(-1);
    }

    printf("[led_app] LED blinking started!! \n");

    // LED ON/OFF 반복 
    for(int i=0; i < 30; ++i){
        // LED 상태 출력 및 ioctl 출력
        printf("[led_app] DMS_LED turn %s\n", (i % 2) ? "ON" : "OFF");
        ioctl(fd, (i % 2) ? DMS_LED_ON : DMS_LED_OFF, 0);
        usleep(300000); // 300ms 대기
    }
    ioctl(fd, DMS_LED_OFF, 0);

    close(fd);
    return 0;

}