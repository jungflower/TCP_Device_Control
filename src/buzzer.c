#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>
#include <stdbool.h>
#include "device_control.h"

#define SPKR BUZZER_PIN

static pthread_t music_thread;
static volatile bool playing = false;    // volatile: 최적화 방지
static volatile bool stop_signal = false;

int notes[] = {
    660, 660, 0, 660, 0, 510, 660, 0,
    770, 0, 0, 0, 380, 0, 0, 0,

    510, 0, 0, 380, 0, 0, 320, 0,
    0, 440, 0, 480, 0, 450, 430, 0,

    380, 660, 760, 860, 0, 700, 770, 0,
    660, 0, 520, 580, 480, 0,

    510, 0, 0, 380, 0, 0, 320, 0,
    0, 440, 0, 480, 0, 450, 430, 0,

    380, 660, 760, 860, 0, 700, 770, 0,
    660, 0, 520, 580, 480, 0
};

int durations[] = {
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,

    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,

    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100,

    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,

    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100
};

int notes2[] = {
    659, 659, 784, 784, 880, 880, 784, 0,
    698, 698, 659, 659, 587, 587, 523, 0,

    784, 784, 698, 698, 659, 659, 587, 0,
    784, 784, 698, 698, 659, 659, 587, 0,

    659, 784, 880, 784, 659, 523, 587, 659,
    523, 659, 784, 659, 523, 0
};

int durations2[] = {
    300, 300, 300, 300, 300, 300, 600, 200,
    300, 300, 300, 300, 300, 300, 600, 200,

    300, 300, 300, 300, 300, 300, 600, 200,
    300, 300, 300, 300, 300, 300, 600, 200,

    300, 300, 300, 300, 300, 300, 300, 600,
    300, 300, 300, 600, 600, 400
};

#define TOTAL (sizeof(notes)/sizeof(notes[0]))
#define TOTAL2 (sizeof(notes2)/sizeof(notes2[0]))

void* musicPlay(void* arg) {
    playing = true;
    stop_signal = false;

    for (int i = 0; i < TOTAL && !stop_signal; ++i) {
        softToneWrite(SPKR, notes[i]);
        delay(durations[i]);
    }

    softToneWrite(SPKR, 0); // 노래 멈춤
    playing = false;
    return NULL;
}

void buzzer_on() {
    if (!playing) {
        pthread_create(&music_thread, NULL, musicPlay, NULL);
    }
}

void buzzer_off() {
    if (playing) {
        stop_signal = true;
        pthread_join(music_thread, NULL);  // 스레드 안전하게 종료 대기
        softToneWrite(SPKR, 0);            // 강제 정지
        playing = false;
    }
}

__attribute__((constructor))
void setup_buzzer() {
    wiringPiSetup();
    pinMode(SPKR, OUTPUT);
    softToneCreate(SPKR);  // 소프트톤은 한 번만 생성
}