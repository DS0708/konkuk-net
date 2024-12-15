#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>

volatile int is_running = 1; 

void* send_message(void* arg){
    int server_sock = *(int*)arg;
    char buffer[1024];
    while(is_running){
        memset(buffer, 0, sizeof(buffer));

        if(fgets(buffer, sizeof(buffer), stdin) != NULL){
            buffer[strlen(buffer)-1] = '\0';
            write(server_sock, buffer, sizeof(buffer));
        }
        if(strncmp(buffer, "exit", 4) == 0){
            printf("서버에서 연결이 끊어졌습니다.\n");
            is_running = 0;
            break;
        }
    }
    close(server_sock);
    return 0;
}

void* receive_message(void* arg){
    int server_sock = *(int*)arg;
    char buffer[1024];
    while(is_running){
        memset(buffer, 0, sizeof(buffer));
        int read_len = read(server_sock, buffer, sizeof(buffer));
        if(read_len > 0){
            printf("Received: %s\n", buffer);
        }
        if(strncmp(buffer, "exit", 4) == 0){
            printf("서버에서 연결이 끊어졌습니다.\n");
            is_running = 0;
            break;
        }
    }
    close(server_sock);
    return 0;
}

int main(int argc, char* argv[]) {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        perror("socket error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    if(connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect error");
        exit(1);
    }
    printf("서버에 연결되었습니다.\n");


    pthread_t send, receive;

    pthread_create(&send, NULL, send_message, (void*)&server_socket);
    pthread_create(&receive, NULL, receive_message, (void*)&server_socket);

    pthread_join(send, NULL);
    pthread_join(receive, NULL);

    return 0;
}

