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
    struct sockaddr_in server_addr;
    int client_socket;
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(client_socket == -1) {
        printf("socket 생성 실패\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(4000);

    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("connect 실패\n");
        exit(1);
    }

    printf("Socket Connection Success!!\n");
    printf("Please Enter the message!!\n");

    while(1) {
        clear_buffer(buff_snd);
        clear_buffer(buff_rcv);

        // 클라이언트 메시지 입력
        printf("client : ");
        if(fgets(buff_snd, BUFF_SIZE, stdin) != NULL){
            int len = strlen(buff_snd);
            if(len > 0 && buff_snd[len-1]=='\n'){
                buff_snd[len-1] = '\0';
                len--;
            }

            if(len > 0){
                int check = write(client_socket, buff_snd, len);
                if(check <= 0) {
                        printf("write error\n");
                        break;
                }
            }
        }

        if(buff_snd[0] == 'q' && strlen(buff_snd) == 1) {
            printf("Closing connection...\n");
            break;
        }

        // 서버로부터 응답 수신
        int read_len = read(client_socket, buff_rcv, BUFF_SIZE - 1);
        if(read_len <= 0) {
            printf("Server disconnected\n");
            break;
        }
        
        printf("server : %s\n", buff_rcv);
    }

    close(client_socket);
    return 0;
}