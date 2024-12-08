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

typedef struct {
    char operator;
    int operand1;
    int operand2;
    int check;
    int result;
} CalcData;


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
    server_addr.sin_port = htons(4001);

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
            CalcData data;
            int read_len = read(client_socket, &data, sizeof(CalcData));

            if(read_len <= 0) {
                printf("Client disconnected\n");
                break;
            }

            // check가 0이면 클라이언트가 종료를 요청한 것
            if(data.check == 0) {
                printf("Client sent termination signal\n");
                break;
            }

            // 수신 데이터 출력
            printf("Received: %d %c %d\n", data.operand1, data.operator, data.operand2);
            
            // 계산 수행
            switch(data.operator) {
                case '+': data.result = data.operand1 + data.operand2; break;
                case '-': data.result = data.operand1 - data.operand2; break;
                case '*': data.result = data.operand1 * data.operand2; break;
                case '/': 
                    if(data.operand2 != 0) { //0으로 나누면 안됨
                        data.result = data.operand1 / data.operand2;
                    } else {
                        data.result = 0;  // 또는 에러 처리
                        printf("Division by zero error\n");
                    }
                    break;
                default: 
                    printf("Invalid operator\n");
                    data.result = 0;
            }

            // 결과 전송
            write(client_socket, &data, sizeof(CalcData));
        }


        close(client_socket);
    }

    close(server_socket);
    return 0;
}