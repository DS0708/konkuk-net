#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

// 동일
int main(int argc, char** argv) {
    int serv_sock = 0;
    int clnt_sock = 0;
    int clnt_addr_size = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    char buff[BUFF_SIZE + 5];
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];

    memset(&serv_addr, 0x00, sizeof(serv_addr));
    memset(&clnt_addr, 0x00, sizeof(clnt_addr));

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) {
        printf("server socket 생성 실패\n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("bind() 실행 에러\n");
        exit(1);
    }

    if(listen(serv_sock, 5) == -1) {
        printf("listen() 실행 실패\n");
        exit(1);
    }

    // 여기부터

    while(1) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

        if(clnt_sock == -1) {
            printf("클라이언트 연결 수락 실패\n");
            exit(1);
        }

        while(1) {
            int ret = read(clnt_sock, buff_rcv, BUFF_SIZE);
            if(ret <= 0) {
                printf("read error\n");
                break;
            }
            printf("receive: %s\n", buff_rcv);
            if(strlen(buff_rcv) == 1 && buff_rcv[0] == 'q') {
                printf("클라이언트 종료 요청\n");
                break;
            }
            scanf("%s", buff_snd);
            ret = write(clnt_sock, buff_snd, strlen(buff_snd) + 1);
            if(ret < 0) {
                printf("write error\n");
                break;
            }
        }
        close(clnt_sock);
    }
}
