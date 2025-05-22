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
#include <stdarg.h>

#define PORT 5100
#define BUF_SIZE 1024

// 뮤텍스 초기화
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;       // 일반 공유 자원 동기화
pthread_mutex_t buzzer_mutex = PTHREAD_MUTEX_INITIALIZER; // 부저 관련 동기화

int server_socket; // 서버 소켓 디스크립터

// 장치 함수 포인터
void (*led_on)();
void (*led_off)();
void (*led_brightness)(int);
void (*buzzer_on)();
void (*buzzer_off)();
int (*auto_led_control_by_light)();
void (*segment_countdown)(int);

/* 서버 로그 남기기 */
// 로그 파일 포인터
FILE *log_fp = NULL;

// 로그 초기화 함수
void log_init(const char *path) {
    log_fp = fopen(path, "w");
    if (!log_fp) {
        perror("log file open error");
        exit(1);
    }
}

// 로그 쓰기 함수 (표준 출력 + 로그 파일 동시 출력)
void log_write(const char *level, const char *fmt, ...) {
    if (!log_fp) return;

    // 현재 시간
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t);

    // 로그 메시지 생성
    char log_msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_msg, sizeof(log_msg), fmt, args);
    va_end(args);

    // level 안넣을 때 
    if(level == NULL){
        fprintf(log_fp, "[%s] %s", time_buf, log_msg);
        fflush(log_fp);
         printf("[%s] %s", time_buf, log_msg);
        return;
    }

    // 출력: [시간] [레벨] 메시지
    fprintf(log_fp, "[%s] [%s] %s", time_buf, level, log_msg); 
    fflush(log_fp);

    // 표준 출력도 동일하게
    printf("[%s] [%s] %s", time_buf, level, log_msg);
}
//////////////////////////////////

// 클라이언트 처리 스레드 함수
void* client_handler(void* arg) {
    int csock = *(int*)arg;
    free(arg);

    char buf[BUF_SIZE];

    while(1){
        int len = read(csock, buf, BUF_SIZE - 1); // 클라이언트로부터 명령 수신
        if (len <= 0) { // 클라이언트 연결 끊김 또는 에러
            break;
        }
        buf[len] = '\0'; // 문자열 종료 

        // 커맨드 로그 출력
        log_write("COMMAND", "%s\n", buf);

        char response[BUF_SIZE] = {0}; // 클라이언트 응답 메세지

        // 명령 분석은 스레드별 독립 작업, mutex 필요 없음
        if (strcmp(buf, "led_on") == 0) {
            pthread_mutex_lock(&mutex);
            led_on();
            pthread_mutex_unlock(&mutex);
            snprintf(response, BUF_SIZE, "🟢 LED turned ON\n");
        }
        else if (strcmp(buf, "led_off") == 0) {
            pthread_mutex_lock(&mutex);
            led_off();
            pthread_mutex_unlock(&mutex);
            snprintf(response, BUF_SIZE, "🔴 LED turned OFF\n");
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
                snprintf(response, BUF_SIZE, "💡 LED brightness set to %d\n", level);
            } else {
                snprintf(response, BUF_SIZE, "❎ Invalid brightness level (0~2 allowed)\n");
            }
        }
        else if (strcmp(buf, "buzzer_on") == 0) {
            // pthread_mutex_lock(&buzzer_mutex);
            // buzzer_on();
            // pthread_mutex_unlock(&buzzer_mutex);
            // snprintf(response, BUF_SIZE, "🎵 Buzzer turned ON\n");
                const char *song_prompt = "🎵 Which song do you want to play? (1 or 2): ";
            write(csock, song_prompt, strlen(song_prompt));

            char song_buf[BUF_SIZE] = {0};
            int song_len = read(csock, song_buf, BUF_SIZE - 1);
            if (song_len <= 0) {
                snprintf(response, BUF_SIZE, "❌ No song selected. Cancelling buzzer_on.\n");
            } else {
                song_buf[song_len] = '\0';
                int song_id = strtol(song_buf, NULL, 10);
                if (song_id == 1 || song_id == 2) {
                    pthread_mutex_lock(&buzzer_mutex);
                    buzzer_on(song_id);  // 수정된 buzzer_on 함수
                    pthread_mutex_unlock(&buzzer_mutex);
                    snprintf(response, BUF_SIZE, "🎵 Buzzer turned ON with song %d\n", song_id);
                } else {
                    snprintf(response, BUF_SIZE, "❎ Invalid song number. Choose 1 or 2.\n");
                }
            }
        }
        else if (strcmp(buf, "buzzer_off") == 0) {
            pthread_mutex_lock(&buzzer_mutex);
            buzzer_off();
            pthread_mutex_unlock(&buzzer_mutex);
            snprintf(response, BUF_SIZE, "🎵 Buzzer turned OFF\n");
        }
        else if (strcmp(buf, "read_light") == 0) {
            pthread_mutex_lock(&mutex);
            int light = auto_led_control_by_light();
            pthread_mutex_unlock(&mutex);
            if(light == 0){ // 빛 있을 때
                snprintf(response, BUF_SIZE, "🌞 light_sensor value: %d -> LED ON\n", light);
            }
            else{ // 빛 없을 때
                snprintf(response, BUF_SIZE, "🌚 light_sensor value: %d -> LED ON\n", light);
            }
            
        }
        else if (strncmp(buf, "countdown ", 10) == 0) {
            char *num_str = buf + 10;
            char *endptr;
            int num = strtol(num_str, &endptr, 10);
            if (*endptr == '\0' && num > 0) {
                pthread_mutex_lock(&buzzer_mutex);
                segment_countdown(num);
                pthread_mutex_unlock(&buzzer_mutex);
                snprintf(response, BUF_SIZE, "📟 7-segment countdown started from %d\n", num);
            } else {
                snprintf(response, BUF_SIZE, "❎ Please enter a positive integer for countdown\n");
            }
        }
        else if (strcmp(buf, "help") == 0) {
            const char* help_msg =
                "Available commands:\n"
                "led_on          : Turn LED ON\n"
                "led_off         : Turn LED OFF\n"
                "led_brightness X: Set LED brightness level (0~2)\n"
                "buzzer_on       : Turn buzzer ON (will ask song number)\n"
                "buzzer_off      : Turn buzzer OFF\n"
                "read_light      : Read light sensor value and control LED\n"
                "countdown N     : Start 7-seg countdown from N\n"
                "quit / exit     : Disconnect client\n"
                "help            : Show this help message\n";
            snprintf(response, BUF_SIZE, "%s", help_msg);
        }
        else {
            snprintf(response, BUF_SIZE, "❌ Unknown command: %.1000s\n", buf);
        }
        // 응답 로그 기록
        log_write("RESPONSE", "%s", response); // \n 포함되어 있음
        write(csock, response, strlen(response));
    }

    close(csock);
    return NULL;
}

int main(int argc, char **argv) {
    // 로그 파일 경로 인자에서 받거나 기본 경로 설정
    const char *log_path = (argc > 1) ? argv[1] : "./tcp_server.log";
    log_init(log_path);

    log_write(NULL, "=== TCP Server Starting ===\n");

    // 시그널 등록
    signal(SIGPIPE, SIG_IGN);  // SIGPIPE 무시 (클라이언트 연결 끊김 시 쓰기 문제 방지)

    // 장치 제어용 공유 라이브러리 로드
    // RTLD_NOW: 로딩 시점에 모든 심볼 즉시 찾아 연결
    // RTLD_GLOBAL: 로드한 심볼들을 전역 심볼 테이블에 등록
    void *led_handle = dlopen("./libled.so", RTLD_NOW | RTLD_GLOBAL); 
    void *buzzer_handle = dlopen("./libbuzzer.so", RTLD_NOW | RTLD_GLOBAL);
    void *light_handle = dlopen("./liblight_sensor.so", RTLD_NOW | RTLD_GLOBAL);
    void *segment_handle = dlopen("./libsegment.so", RTLD_NOW | RTLD_GLOBAL);

    // 라이브러리 로드 실패 처리
    if (!led_handle || !buzzer_handle || !light_handle || !segment_handle) {
        perror("dlopen error");
        log_write("INFO", "dlopen error\n");
        exit(1);
    }

    // 함수 주소 매핑 (dlsym)
    led_on = dlsym(led_handle, "led_on");
    led_off = dlsym(led_handle, "led_off");
    led_brightness = dlsym(led_handle, "led_set_brightness");
    buzzer_on = dlsym(buzzer_handle, "buzzer_on");
    buzzer_off = dlsym(buzzer_handle, "buzzer_off");
    auto_led_control_by_light = dlsym(light_handle, "auto_led_control_by_light");
    segment_countdown = dlsym(segment_handle, "segment_countdown");

    // 서버 소켓 생성
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        log_write("INFO", "socket creation failed\n");
        exit(1);
    }

    // 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // 바인딩 
    // 클라이언트로부터 들어오는 서비스를 현재 어플리케이션이 사용할 수 있도록 해당 포트를 OS에 등록하는 작업 
    if (bind(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        log_write("INFO","bind failed\n");
        exit(1);
    }

    // 클라이언트 연결 대기 (5명의 클라이언트까지만 받기)
    if (listen(server_socket, 5) < 0) {
        perror("listen");
        log_write("INFO","listen failed\n");
        exit(1);
    }

    log_write(NULL,"This server is listening on port [%d]\n", PORT);

    // 클라이언트 연결 처리 루프
    while (1) {
        cli_len = sizeof(cli_addr);
        int *csock = malloc(sizeof(int));
        if (!csock) {
            perror("malloc");
            log_write("INFO","malloc failed\n");
            continue;
        }
        // 클라이언트 연결 받음
        *csock = accept(server_socket, (struct sockaddr *)&cli_addr, &cli_len);
        if (*csock < 0) {
            perror("accept");
            log_write("INFO","accept failed\n");
            free(csock);
            continue;
        }

        // 클라이언트 IP, 포트 출력
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        log_write("INFO","Client connected from %s:[%d]\n", client_ip, ntohs(cli_addr.sin_port));

        pthread_t tid;
        int ret = pthread_create(&tid, NULL, client_handler, csock);
        if (ret != 0) {
            perror("pthread_create");
            log_write("INFO","pthread_create failed\n");
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