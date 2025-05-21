// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define SERVER_PORT 5100

#if 0
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_ip> <command> [arg]\n", argv[0]);
        fprintf(stderr, "Commands: led_on, led_off, led_brightness <0-2>, buzzer_on, buzzer_off, auto_led, countdown <num>\n");
        return 1;
    }

    char *server_ip = argv[1];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    // 명령어 생성: argv[2] 부터 모두 붙여서 하나의 문자열로
    char send_buf[BUF_SIZE] = {0};
    for (int i = 2; i < argc; i++) {
        strncat(send_buf, argv[i], BUF_SIZE - strlen(send_buf) - 1);
        if (i < argc - 1) {
            strncat(send_buf, " ", BUF_SIZE - strlen(send_buf) - 1);
        }
    }

    write(sock, send_buf, strlen(send_buf));

    char recv_buf[BUF_SIZE] = {0};
    int len = read(sock, recv_buf, BUF_SIZE - 1);
    if (len > 0) {
        recv_buf[len] = '\0';
        printf("서버 응답: %s", recv_buf);
    } else {
        printf("서버로부터 응답이 없습니다.\n");
    }

    close(sock);
    return 0;
}
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("Connected to server. Enter commands. Type 'quit' or 'exit' to quit.\n");

    while (1) {
        char send_buf[BUF_SIZE] = {0};
        printf("> ");

        if (fgets(send_buf, BUF_SIZE, stdin) == NULL) {
            printf("Input error\n");
            break;
        }

        // Remove trailing newline
        send_buf[strcspn(send_buf, "\n")] = '\0';

        // Exit conditions
        if (strcmp(send_buf, "quit") == 0 || strcmp(send_buf, "exit") == 0) {
            printf("Exiting.\n");
            break;
        }

        // Skip empty input
        if (strlen(send_buf) == 0) {
            continue;
        }

        if (write(sock, send_buf, strlen(send_buf)) < 0) {
            perror("write");
            break;
        }

        char recv_buf[BUF_SIZE] = {0};
        int len = read(sock, recv_buf, BUF_SIZE - 1);
        if (len > 0) {
            recv_buf[len] = '\0';
            printf("Server response: %s\n", recv_buf);
        } else {
            printf("No response from server.\n");
        }
    }

    close(sock);
    return 0;
}