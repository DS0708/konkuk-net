#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BUFF_SIZE 1024

int main(int argc, char **argv) {
    int server_socket = 0;
    int client_socket = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];

    // 서버 소켓 설정
    memset(&server_addr, 0, sizeof(server_addr));  // 주소 구조체 초기화
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("server socket 생성 실패");
        exit(1);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET; //Ipv4
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0/0 IP 허용

    // 바인드 에러 처리
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() 실행 에러");
        exit(1);
    }

    // 서버가 한 번에 최대 5개의 동시 연결 요청을 대기 큐에 보관할 수 있음을 의미
    if (listen(server_socket, 5) == -1) {
        perror("listen() 실행 실패");
        exit(1);
    }

    // 클라이언트 요청 처리 루프
    while (1) {
        client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1) {
            perror("클라이언트 연결 수락 실패");
            continue;  // 실패시 다음 연결 대기
        }

        printf("New client connected\n");

        while (1) {
            ssize_t num_bytes_read = read(client_socket, buff_rcv, BUFF_SIZE);
            if (num_bytes_read < 0) {
                perror("read failed");
                strcpy(buff_snd, "fail");
                write(client_socket, buff_snd, strlen(buff_snd) + 1);
                continue;
            }

            if (num_bytes_read == 0) {
                printf("Client disconnected\n");
                break;  // 클라이언트 연결 종료
            }

            printf("receive: %.*s\n", (int)num_bytes_read, buff_rcv);

            if (strncmp(buff_rcv, "bye", 3) == 0) {
                printf("Client said bye. Closing this connection.\n");
                close(client_socket);
                break;  // 현재 클라이언트 연결 종료 후 다음 클라이언트 대기
            }

            // 클라이언트에게 응답 전송
            sprintf(buff_snd, "success : %s", buff_rcv);
            write(client_socket, buff_snd, strlen(buff_snd) + 1);
        }
    }
    
    // 이 부분은 while(1) 루프 때문에 실행되지 않습니다.
    // 서버를 정상적으로 종료하려면 별도의 종료 조건이 필요합니다.
    close(server_socket);
    return 0;
}