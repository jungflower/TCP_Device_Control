#include <stdio.h>
#include <wiringPi.h>
#include "device_control.h"

int read_light_sensor() {
    return digitalRead(LIGHT_SENSOR_PIN);  // 0: 빛 없음, 1: 빛 있음
}

void gled_on() {
    digitalWrite(LED_PIN, 0);  // full brightness
}

void gled_off() {
    digitalWrite(LED_PIN, 100);
}

int auto_led_control_by_light() {
    int light = read_light_sensor();

    if (light == 0) {
        printf("Light off: %d → LED ON\n", light);
        gled_on();
    } else {
        printf("Light on: %d → LED OFF\n", light);
        gled_off();
    }

    return light;
}

__attribute__((constructor))
void setup_light_sensor() {
    wiringPiSetup();
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);  
}