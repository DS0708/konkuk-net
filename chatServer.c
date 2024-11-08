#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

void clear_buffer(char *buffer) {
    memset(buffer, 0, BUFF_SIZE);
}

int main() {
    int server_socket = 0;
    int client_socket = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) {
        printf("socket 생성 실패\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(4000);

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("bind 실패\n");
        exit(1);
    }

    if(listen(server_socket, 5) == -1) {
        printf("listen 실패\n");
        exit(1);
    }

    printf("Chat server started. Waiting for connections...\n");

    while(1) {
        socklen_t client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

        if(client_socket == -1) {
            printf("accept 실패\n");
            continue;
        }

        printf("New client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        while(1) {
            clear_buffer(buff_rcv);
            clear_buffer(buff_snd);

            // 클라이언트로부터 메시지 수신
            int read_len = read(client_socket, buff_rcv, BUFF_SIZE - 1);
            if(read_len <= 0) {
                printf("Client disconnected\n");
                break;
            }

            printf("client : %s\n", buff_rcv);

            if(buff_rcv[0] == 'q' && strlen(buff_rcv) == 1) {
                printf("Client want to quit\n");
                break;
            }

            // 서버 메시지 입력
            printf("server : ");
            if (fgets(buff_snd, BUFF_SIZE, stdin) != NULL) {
                size_t len = strlen(buff_snd);
                // 개행 문자 제거
                if (len > 0 && buff_snd[len-1] == '\n') {
                    buff_snd[len-1] = '\0';
                    len--;
                }
                
                // 빈 문자열이 아닌 경우에만 전송
                if (len > 0) {
                    int write_len = write(client_socket, buff_snd, len);
                    if(write_len <= 0) {
                        printf("write error\n");
                        break;
                    }
                }
            }
        }

        close(client_socket);
    }

    close(server_socket);
    return 0;
}