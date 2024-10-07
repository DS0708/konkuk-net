#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#define BUFF_SIZE 1024

int main(int argc, char **argv) {
    int server_socket = 0;
    int client_socket = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];
    fd_set readfds;
    int max_sd;

    // 서버 소켓 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("server socket 생성 실패");
        exit(1);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 바인드 에러 처리
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() 실행 에러");
        exit(1);
    }

    // 리스닝
    if (listen(server_socket, 5) == -1) {
        perror("listen() 실행 실패");
        exit(1);
    }

    printf("서버가 시작되었습니다. 클라이언트의 연결을 기다리는 중...\n");

    while (1) {
        client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1) {
            perror("클라이언트 연결 수락 실패");
            continue;
        }

        printf("New client connected. \n");

        while (1) {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(client_socket, &readfds);
            max_sd = (STDIN_FILENO > client_socket) ? STDIN_FILENO : client_socket;

            if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
                perror("select error");
                exit(1);
            }

            // 클라이언트로부터 메시지 수신
            if (FD_ISSET(client_socket, &readfds)) {
                ssize_t num_bytes_read = read(client_socket, buff_rcv, BUFF_SIZE);
                if (num_bytes_read <= 0) {
                    if (num_bytes_read == 0) {
                        printf("클라이언트가 연결을 종료했습니다.\n");
                    } else {
                        perror("read failed");
                    }
                    close(client_socket);
                    break;
                }

                printf("클라이언트로부터 받은 메시지: %.*s\n", (int)num_bytes_read, buff_rcv);

                if (strncmp(buff_rcv, "bye", 3) == 0) {
                    printf("클라이언트가 'bye'를 보냈습니다. 연결을 종료합니다.\n");
                    close(client_socket);
                    break;
                }
            }

            // 서버 키보드 입력
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                if (fgets(buff_snd, BUFF_SIZE, stdin) != NULL) {
                    size_t len = strlen(buff_snd);
                    if (len > 0 && buff_snd[len-1] == '\n') {
                        buff_snd[len-1] = '\0';
                    }
                    write(client_socket, buff_snd, strlen(buff_snd));
                    printf("서버 응답 전송: %s\n", buff_snd);
                }
            }
        }
    }
    
    close(server_socket);
    return 0;
}