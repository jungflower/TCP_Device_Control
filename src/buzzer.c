#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "device_control.h"

#define SPKR BUZZER_PIN
#define MAX_QUEUE 10

typedef struct {
    int song_id;
} SongRequest;

static SongRequest song_queue[MAX_QUEUE];
static int queue_front = 0;
static int queue_rear = 0;

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

static volatile bool playing = false;
static volatile bool stop_signal = false;
static bool stop_all = false;

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
    262, 262, 294, 262, 349, 330,
    262, 262, 294, 262, 392, 349,
    262, 262, 523, 440, 349, 330, 294,
    466, 466, 440, 349, 392, 349
};

int durations2[] = {
    150, 150, 300, 300, 300, 600,
    150, 150, 300, 300, 300, 600,
    150, 150, 300, 300, 300, 300, 600,
    150, 150, 300, 300, 300, 600
};

#define TOTAL (sizeof(notes)/sizeof(notes[0]))
#define TOTAL2 (sizeof(notes2)/sizeof(notes2[0]))

// ----------------------------------------
// Queue functions
// ----------------------------------------
void enqueue_song(int song_id) {
    pthread_mutex_lock(&queue_mutex);
    int next = (queue_rear + 1) % MAX_QUEUE;
    if (next != queue_front) {
        song_queue[queue_rear].song_id = song_id;
        queue_rear = next;
        pthread_cond_signal(&queue_not_empty);
    } else {
        printf("Song queue full, ignoring request.\n");
    }
    pthread_mutex_unlock(&queue_mutex);
}

int dequeue_song() {
    pthread_mutex_lock(&queue_mutex);
    while (queue_front == queue_rear) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    int song_id = song_queue[queue_front].song_id;
    queue_front = (queue_front + 1) % MAX_QUEUE;
    pthread_mutex_unlock(&queue_mutex);
    return song_id;
}

// ----------------------------------------
// Song Playback Thread
// ----------------------------------------
void* song_queue_handler(void* arg) {
    while (!stop_all) {
        int song_id = dequeue_song();

        playing = true;
        stop_signal = false;

        if (song_id == 1) {
            for (int i = 0; i < TOTAL && !stop_signal; ++i) {
                softToneWrite(SPKR, notes[i]);
                delay(durations[i]);
            }
        } else if (song_id == 2) {
            for (int i = 0; i < TOTAL2 && !stop_signal; ++i) {
                softToneWrite(SPKR, notes2[i]);
                delay(durations2[i]);
            }
        }

        softToneWrite(SPKR, 0);
        playing = false;
    }
    return NULL;
}

// ----------------------------------------
// Public API
// ----------------------------------------
void buzzer_on(int song_number) {
    enqueue_song(song_number);
}

void buzzer_off() {
    if (playing) {
        stop_signal = true;
        softToneWrite(SPKR, 0);
    }
}

__attribute__((constructor))
void setup_buzzer() {
    wiringPiSetup();
    pinMode(SPKR, OUTPUT);
    softToneCreate(SPKR);

    pthread_t handler_thread;
    pthread_create(&handler_thread, NULL, song_queue_handler, NULL);
    pthread_detach(handler_thread);  // 백그라운드 실행
}