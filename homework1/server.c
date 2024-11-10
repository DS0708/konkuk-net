#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

typedef struct{
    char file_name[100];
} FILEREQUEST;

typedef struct {
    long size;
    char created_time[20];
    char message[100];
} FILE_INFO;

// 파일 존재 여부 확인 및 파일 정보 반환 함수
FILE_INFO get_file_info(FILEREQUEST request) {
    FILE_INFO file_info;
    memset(&file_info, 0, sizeof(FILE_INFO));
    struct stat file_stat;

    // 파일이 존재하는지 확인
    if (stat(request.file_name, &file_stat) == 0) {
        // 파일이 존재하는 경우 정보 설정
        file_info.size = file_stat.st_size;
        strftime(file_info.created_time, sizeof(file_info.created_time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));

        // 파일 내용 읽기
        FILE *file = fopen(request.file_name, "r");
        if (file != NULL) {
            // 최대 100 바이트까지 파일 내용을 읽음
            fread(file_info.message, 1, sizeof(file_info.message) - 1, file);
            file_info.message[sizeof(file_info.message) - 1] = '\0';  // null-terminate
            fclose(file);
        } else {
            snprintf(file_info.message, sizeof(file_info.message), "File found: %s (unable to read content)", request.file_name);
        }
    } else {
        // 파일이 없는 경우 "File Not Found" 메시지 설정
        file_info.size = 0;
        strcpy(file_info.created_time, "N/A");
        strcpy(file_info.message, "File Not Found");
    }

    return file_info;
}


int main() {
    int server_socket = 0;
    int client_socket = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

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

    printf("server started. Waiting for connections...\n");

    while(1) {
        socklen_t client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

        if(client_socket == -1) {
            printf("accept 실패\n");
            continue;
        }

        printf("New client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        while(1) {
            FILEREQUEST request;
            int read_len = read(client_socket, &request, sizeof(FILEREQUEST));

            if(read_len <= 0) {
                printf("Client disconnected\n");
                break;
            }

            // 파일이름이 exit면 종료를 요청한 것
            if (strcmp(request.file_name, "exit") == 0) {
                printf("Client sent termination signal\n");
                printf("\n");
                break;
            }
            
            //파일 찾기
            FILE_INFO result = get_file_info(request);

            // 결과 출력
            printf("File Name: %s\n", request.file_name);
            printf("Size: %ld bytes\n", result.size);
            printf("Created Time: %s\n", result.created_time);
            printf("Message: %s\n", result.message);
            printf("\n");

            // 결과 전송
            write(client_socket, &result, sizeof(FILE_INFO));
        }

        close(client_socket);
    }

    close(server_socket);
    return 0;
}