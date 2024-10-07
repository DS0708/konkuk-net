#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BUFF_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buff[BUFF_SIZE+5];

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket 생성 실패");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Sever Connection Fail");
        exit(1);
    }

    printf("Message :");

    while (1) {
        printf("Message: ");
        fgets(buff, BUFF_SIZE, stdin); 
        buff[strcspn(buff, "\n")] = 0;

        // if (strcmp(buff, "exit") == 0) {
        //     break;  // "exit" 입력 시 종료
        // }

        if (write(client_socket, buff, strlen(buff) + 1) == -1) {
            perror("메시지 전송 실패");
            break;
        }

        ssize_t num_bytes_read = read(client_socket, buff, BUFF_SIZE);
        if (num_bytes_read == -1) {
            perror("메시지 수신 실패");
            break;
        }

        buff[num_bytes_read] = '\0'; // NULL-termination to safely print
        printf("Server Response : %s\n", buff);
    }

    close(client_socket);
    return 0;
}
