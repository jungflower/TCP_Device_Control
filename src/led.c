#include <wiringPi.h>
#include "device_control.h"

// 하드웨어 PWM은 WiringPi 핀 1번 (BCM 18번)만 지원
#define LED_PWM_PIN 1  // WiringPi 기준

void led_on() {
    pwmWrite(LED_PWM_PIN, 1024);  // 최대 duty cycle
}

void led_off() {
    pwmWrite(LED_PWM_PIN, 0);
}

void led_set_brightness(int level) {
    int value;
    switch (level) {
        case 0: value = 300; break;  // low
        case 1: value = 700; break;  // mid
        case 2: value = 1024; break; // high
        default: value = 1024; break;
    }
    pwmWrite(LED_PWM_PIN, value);
}

__attribute__((constructor))
void setup_led() {
    wiringPiSetup();
    pinMode(LED_PWM_PIN, PWM_OUTPUT);  // 하드웨어 PWM 모드로 설정
}