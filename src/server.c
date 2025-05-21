// src/server_multithread.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 5100
#define BUF_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;       // 일반 공유 자원 동기화
pthread_mutex_t buzzer_mutex = PTHREAD_MUTEX_INITIALIZER; // 부저 관련 동기화

int server_socket;

// 장치 함수 포인터
void (*led_on)();
void (*led_off)();
void (*led_brightness)(int);
void (*buzzer_on)();
void (*buzzer_off)();
void (*auto_led_control_by_light)();
void (*segment_countdown)(int);

// 클라이언트 처리 스레드 함수
void* client_handler(void* arg) {
    int csock = *(int*)arg;
    free(arg);

    char buf[BUF_SIZE];

    while(1){
        int len = read(csock, buf, BUF_SIZE - 1);
        if (len <= 0) { // 클라이언트 연결 끊김 또는 에러
            break;
        }
        buf[len] = '\0';

        char response[BUF_SIZE] = {0};

        // 명령 분석은 스레드별 독립 작업, mutex 필요 없음
        if (strcmp(buf, "led_on") == 0) {
            pthread_mutex_lock(&mutex);
            led_on();
            pthread_mutex_unlock(&mutex);
            snprintf(response, BUF_SIZE, "LED turned ON\n");
        }
        else if (strcmp(buf, "led_off") == 0) {
            pthread_mutex_lock(&mutex);
            led_off();
            pthread_mutex_unlock(&mutex);
            snprintf(response, BUF_SIZE, "LED turned OFF\n");
        }
        else if (strncmp(buf, "led_brightness ", 15) == 0) {
            // 숫자 파싱 및 유효성 검사
            char *level_str = buf + 15;
            char *endptr;
            int level = strtol(level_str, &endptr, 10);
            if (*endptr == '\0' && level >= 0 && level <= 2) {
                pthread_mutex_lock(&mutex);
                led_brightness(level);
                pthread_mutex_unlock(&mutex);
                snprintf(response, BUF_SIZE, "LED brightness set to %d\n", level);
            } else {
                snprintf(response, BUF_SIZE, "Invalid brightness level (0~2 allowed)\n");
            }
        }
        else if (strcmp(buf, "buzzer_on") == 0) {
            pthread_mutex_lock(&buzzer_mutex);
            buzzer_on();
            pthread_mutex_unlock(&buzzer_mutex);
            snprintf(response, BUF_SIZE, "Buzzer turned ON\n");
        }
        else if (strcmp(buf, "buzzer_off") == 0) {
            pthread_mutex_lock(&buzzer_mutex);
            buzzer_off();
            pthread_mutex_unlock(&buzzer_mutex);
            snprintf(response, BUF_SIZE, "Buzzer turned OFF\n");
        }
        else if (strcmp(buf, "auto_led") == 0) {
            pthread_mutex_lock(&mutex);
            auto_led_control_by_light();
            pthread_mutex_unlock(&mutex);
            snprintf(response, BUF_SIZE, "Auto LED control by light sensor executed\n");
        }
        else if (strncmp(buf, "countdown ", 10) == 0) {
            char *num_str = buf + 10;
            char *endptr;
            int num = strtol(num_str, &endptr, 10);
            if (*endptr == '\0' && num > 0) {
                pthread_mutex_lock(&buzzer_mutex);
                segment_countdown(num);
                pthread_mutex_unlock(&buzzer_mutex);
                snprintf(response, BUF_SIZE, "7-segment countdown started from %d\n", num);
            } else {
                snprintf(response, BUF_SIZE, "Please enter a positive integer for countdown\n");
            }
        }
        else {
            snprintf(response, BUF_SIZE, "Unknown command: %s\n", buf);
        }

        write(csock, response, strlen(response));
    }

    close(csock);
    return NULL;
}

// 시그널 핸들러 (서버 종료)
void handle_sigint(int sig) {
    printf("\n서버 종료 중...\n");
    close(server_socket);
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, SIG_IGN);  // SIGPIPE 무시 (클라이언트 연결 끊김 시 쓰기 문제 방지)

    // 라이브러리 로드
    // RTLD_NOW: 로딩 시점에 모든 심볼 즉시 찾아 연결
    // RTLD_GLOBAL: 로드한 심볼들을 전역 심볼 테이블에 등록
    void *led_handle = dlopen("./libled.so", RTLD_NOW | RTLD_GLOBAL); 
    void *buzzer_handle = dlopen("./libbuzzer.so", RTLD_NOW | RTLD_GLOBAL);
    void *light_handle = dlopen("./liblight_sensor.so", RTLD_NOW | RTLD_GLOBAL);
    void *segment_handle = dlopen("./libsegment.so", RTLD_NOW | RTLD_GLOBAL);

    if (!led_handle || !buzzer_handle || !light_handle || !segment_handle) {
        perror("dlopen error");
        exit(1);
    }

    led_on = dlsym(led_handle, "led_on");
    led_off = dlsym(led_handle, "led_off");
    led_brightness = dlsym(led_handle, "led_set_brightness");
    buzzer_on = dlsym(buzzer_handle, "buzzer_on");
    buzzer_off = dlsym(buzzer_handle, "buzzer_off");
    auto_led_control_by_light = dlsym(light_handle, "auto_led_control_by_light");
    segment_countdown = dlsym(segment_handle, "segment_countdown");

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("서버가 %d 포트에서 실행 중...\n", PORT);

    while (1) {
        cli_len = sizeof(cli_addr);
        int *csock = malloc(sizeof(int));
        if (!csock) {
            perror("malloc");
            continue;
        }

        *csock = accept(server_socket, (struct sockaddr *)&cli_addr, &cli_len);
        if (*csock < 0) {
            perror("accept");
            free(csock);
            continue;
        }

        // 클라이언트 IP, 포트 출력
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("클라이언트 접속: %s:%d\n", client_ip, ntohs(cli_addr.sin_port));

        pthread_t tid;
        int ret = pthread_create(&tid, NULL, client_handler, csock);
        if (ret != 0) {
            perror("pthread_create");
            close(*csock);
            free(csock);
            continue;
        }
        pthread_detach(tid);
    }

    // 프로그램 종료 시 dlclose 해주기 (도달하지 않음, 참고용)
    dlclose(led_handle);
    dlclose(buzzer_handle);
    dlclose(light_handle);
    dlclose(segment_handle);

    close(server_socket);
    return 0;
}