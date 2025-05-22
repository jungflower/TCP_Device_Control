// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUF_SIZE 1024
#define SERVER_PORT 5100

int sock = -1;  // 전역으로 둬서 signal handler에서 접근 가능하게 함

void sigint_handler(int signo) {
    if (sock < 0) {
        exit(1);
    }

    // 서버에 종료 명령 전송 (예: "quit")
    const char *quit_msg = "quit";
    write(sock, quit_msg, strlen(quit_msg));
    printf("\nSIGINT received, sent quit command to server, exiting client.\n");

    close(sock);
    exit(0);
}

void ignore_signal(int signo) {
    // 아무 행동도 하지 않고 시그널 무시
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];

    sock = socket(AF_INET, SOCK_STREAM, 0);
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

    // signal 함수로 시그널 핸들러 등록
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, ignore_signal);
    signal(SIGQUIT, ignore_signal);
    signal(SIGHUP, ignore_signal);

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