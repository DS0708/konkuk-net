#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BUFF_SIZE 1024

int main(int argc, char **argv) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buff[BUFF_SIZE+5];

    if (argc < 2) {
        fprintf(stderr, "사용법: %s <메시지>\n", argv[0]);
        exit(1);
    }

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket 생성 실패");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("서버 접속 실패");
        exit(1);
    }

    if (write(client_socket, argv[1], strlen(argv[1]) + 1) == -1) {
        perror("메시지 전송 실패");
        close(client_socket);
        exit(1);
    }

    ssize_t num_bytes_read = read(client_socket, buff, BUFF_SIZE);
    if (num_bytes_read == -1) {
        perror("메시지 수신 실패");
        close(client_socket);
        exit(1);
    }

    buff[num_bytes_read] = '\0'; // NULL-termination to safely print
    printf("%s\n", buff);
    close(client_socket);
    return 0;
}
