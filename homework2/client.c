#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int max_sd;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT);

    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("Connect fail\n");
        exit(1);
    }

    printf("Connected to the server at localhost:%d\n", SERVER_PORT);
    printf("Enter 'bye' to disconnect.\n\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client_socket, &readfds);
        max_sd = client_socket;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
            break;
        }

        if (FD_ISSET(client_socket, &readfds)) {
            int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (valread <= 0) {
                printf("Server disconnected or error occurred\n");
                break;
            }
            buffer[valread] = '\0';
            printf("Server: %s", buffer);

            if(strncmp(buffer,"bye",3)==0){
                printf("Connection has been disconnected by the server.\n");
                break;
            }
            else if (strcmp(buffer, "all bye\n") == 0) {
                printf("Connection has been disconnected by the server.\n");
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                continue;
            }
            if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
                perror("Send failed");
                break;
            }
            if(strncmp(buffer,"bye",3)==0){
                printf("Connection has been disconnected by the client.\n");
                break;
            }
        }
    }

    close(client_socket);
    return 0;
}
