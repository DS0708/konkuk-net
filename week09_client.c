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
    struct sockaddr_in server_addr;
    int client_socket;
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];
    CalcData data;

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(client_socket == -1) {
        printf("socket 생성 실패\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(4001);

    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("connect 실패\n");
        exit(1);
    }

    printf("Socket Connection Success!!\n");
    printf("Please Enter the message!!\n");

    while (1) {
        printf("client : ");
        if (fgets(buff_snd, BUFF_SIZE, stdin) != NULL) {
            // 개행문자 제거
            buff_snd[strcspn(buff_snd, "\n")] = 0;

            // bye 입력 시 종료 처리
            if(strcmp(buff_snd, "bye") == 0) {
                data.check = 0;  // 종료 신호
                write(client_socket, &data, sizeof(CalcData));
                printf("Closing connection...\n");
                break;
            }

            // 정상적인 계산식 처리
            data.check = 1;  // 계속 진행
            if (sscanf(buff_snd, "%d %c %d", &data.operand1, &data.operator, &data.operand2) == 3) {
                // 구조체 전송
                write(client_socket, &data, sizeof(CalcData));

                // 서버로부터 결과 수신
                CalcData result;
                int read_len = read(client_socket, &result, sizeof(CalcData));
                if (read_len <= 0) {
                    printf("Server disconnected\n");
                    break;
                }
                printf("server : %d\n", result.result);
            } else {
                printf("Invalid format! Please use: number operator number (ex: 3 + 5)\n");
            }
        }
    }

    close(client_socket);
    return 0;
}