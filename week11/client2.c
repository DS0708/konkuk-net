#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

void clear_buffer(char *buffer) {
    memset(buffer, 0, BUFF_SIZE);
}

int main() {
    int sock = 0;
    struct sockaddr_in server_addr;
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];
    
    // 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        printf("socket 생성 실패\n");
        exit(1);
    }
    
    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost
    server_addr.sin_port = htons(4000);
    
    // 서버에 연결
    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("connect 실패\n");
        exit(1);
    }
    
    printf("서버에 연결되었습니다.\n");
    
    // 게임 시작 메시지 수신
    clear_buffer(buff_rcv);
    int read_len = read(sock, buff_rcv, BUFF_SIZE - 1);
    if(read_len > 0) {
        printf("서버: %s\n", buff_rcv);
    }
    
    // 게임 진행
    while(1) {
        clear_buffer(buff_snd);
        clear_buffer(buff_rcv);
        
        // 사용자 입력 받기
        printf("\n숫자를 입력하세요 (1-100): ");
        if(fgets(buff_snd, BUFF_SIZE, stdin) != NULL) {
            size_t len = strlen(buff_snd);
            if(len > 0 && buff_snd[len-1] == '\n') {
                buff_snd[len-1] = '\0';
                len--;
            }
            
            // 서버로 전송
            write(sock, buff_snd, len);
            
            // 서버 응답 수신
            read_len = read(sock, buff_rcv, BUFF_SIZE - 1);
            if(read_len <= 0) {
                printf("서버와 연결이 끊어졌습니다.\n");
                break;
            }
            
            printf("서버: %s\n", buff_rcv);
            
            // 정답을 맞췄는지 확인
            if(strstr(buff_rcv, "정답입니다") != NULL) {
                printf("게임을 종료합니다!\n");
                break;
            }
        }
    }
    
    close(sock);
    return 0;
}