#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

#define LED_PWM_PIN 1  // BCM 18 = wiringPi 1
#define BUZZER_PIN 25  // BCM 26 = wiringPi 25
#define LIGHT_SENSOR_PIN 3  // BCM 22 = wiringPi 3
#define LED_PIN 2  // BCM 27 = wiringPi 2 (조도센서 LED)

// LED
void led_on();
void led_off();
void led_set_brightness(int level);  // 0=low, 1=mid, 2=high

// BUZZER
void* musicPlay(void* arg);
void buzzer_on();
void buzzer_off();

// LIGHT SENSOR
int read_light_sensor();
int auto_led_control_by_light();

// SEGMENT
void segment_display(int num);  // 0~9
void segment_countdown(int num);  // countdown with 1sec interval

#endif