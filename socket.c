#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int server_socket;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        printf("서버 소켓 생성 실패\n");
        exit(1);
    }

    return 0;
}
