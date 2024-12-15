#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>


int main(int argc, char* argv[]) {
    int server_socket;
    struct sockaddr_in server_addr;
    fd_set readfds;
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        perror("socker error");
        exit(1);
    }

    memset(&server_addr,0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed\n");
        exit(1);
    }

    if(listen(server_socket, 5) < 0){
        perror("listen error");
        exit(1);
    }

    while(1){
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        int client_sock = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_sock < 0){
            perror("accept error");
            continue;
        }
        printf("clinet connected\n");

        FD_ZERO(&readfds);
        
    }

    return 0;
}

