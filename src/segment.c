#include <wiringPi.h>
#include <unistd.h>
#include "device_control.h"

int fnd_pins[4]={4,5,16,15}; /*D BCM4 C BCM24 B BCM 16 A BCM 15*/
const int segment_map[10][4] = {
    {0,0,0,0}, // 0
    {0,0,0,1}, // 1
    {0,0,1,0}, // 2
    {0,0,1,1}, // 3
    {0,1,0,0}, // 4
    {0,1,0,1}, // 5
    {0,1,1,0}, // 6
    {0,1,1,1}, // 7
    {1,0,0,0}, // 8
    {1,0,0,1}  // 9
};

void segment_display(int num) {
    if (num < 0 || num > 9) return;
    
    for(int i=0; i < 4; ++i){
        digitalWrite(fnd_pins[i], segment_map[num][i]);
    }
}

void segment_countdown(int num) {
   if (num < 0 || num > 9) return;

    segment_display(num);  // 숫자 바로 표시
    sleep(1);              // 1초 기다린 후 감소 시작

    for (int i = num - 1; i >= 0; --i) {
        segment_display(i);
        sleep(1);
    }

    buzzer_on(1);
    sleep(1);
    buzzer_off();
}

__attribute__((constructor))
void setup_segment() {
    wiringPiSetup();
    for(int i=0; i < 4; ++i){
        pinMode(fnd_pins[i], OUTPUT);
    }    
    segment_display(0);  // 초기값
}