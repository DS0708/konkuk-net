#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#define BUFF_SIZE 1024

typedef struct{
    char file_name[100];
} FILEREQUEST;

typedef struct {
    long size;
    char created_time[20];
    char message[100];
} FILE_INFO;


int main() {
    struct sockaddr_in server_addr;
    int client_socket;
    char buff_snd[BUFF_SIZE];
    FILEREQUEST request;
    FILE_INFO result;

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

    while (1) {
        // buff_snd 초기화
        memset(buff_snd, 0, BUFF_SIZE);

        //사용자 입력
        printf("File Name : ");

        if (fgets(buff_snd, BUFF_SIZE, stdin) != NULL) {
            // 개행문자 제거
            buff_snd[strcspn(buff_snd, "\n")] = 0;

            // request 초기화 및 파일 이름 복사
            memset(&request, 0, sizeof(FILEREQUEST));
            strcpy(request.file_name, buff_snd);

            // exit 입력 시 종료 처리
            if(strcmp(buff_snd, "exit") == 0) {
                write(client_socket, &request, sizeof(FILEREQUEST));
                printf("Closing connection...\n");
                break;
            }

            // 정상적 처리
            //request 전송
            write(client_socket, &request, sizeof(FILEREQUEST));

            // result 구조체 초기화
            memset(&result, 0, sizeof(FILE_INFO));

            // 서버로부터 결과 수신
            int read_len = read(client_socket, &result, sizeof(FILE_INFO));

            //예외 처리
            if (read_len <= 0) {
                    printf("Server disconnected\n");
                    break;
                }

            // 결과 출력
            if(result.size==0){ //파일이 없을때
                printf("Message: %s\n", result.message);
                printf("\n");
            }else{ //파일이 존재할 때
                printf("Size: %ld bytes\n", result.size);
                printf("Created Time: %s\n", result.created_time);
                printf("Message: %s\n", result.message);
                printf("\n");
            }
        }
    }

    close(client_socket);
    return 0;
}