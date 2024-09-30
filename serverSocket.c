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
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 바인드 에러 처리
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() 실행 에러");
        exit(1);
    }

    // 리슨 에러 처리
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

        // 클라이언트로부터 데이터 읽기
        ssize_t num_bytes_read = read(client_socket, buff_rcv, BUFF_SIZE);
        if (num_bytes_read < 0) {
            perror("read failed");
            close(client_socket);
            continue;  // 읽기 실패 시 다음 연결 대기
        }

        printf("receive: %.*s\n", (int)num_bytes_read, buff_rcv);  // 안전하게 출력

        // 클라이언트에게 응답 전송
        sprintf(buff_snd, "%ld : %s", num_bytes_read, buff_rcv);
        write(client_socket, buff_snd, strlen(buff_snd) + 1);  // NULL 포함 전송

        close(client_socket);  // 클라이언트 소켓 닫기
    }

    return 0;  // 이 부분은 서버가 종료될 경우에 도달합니다.
}
