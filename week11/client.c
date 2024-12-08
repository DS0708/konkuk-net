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

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buff_snd[BUFF_SIZE];
    char buff_rcv[BUFF_SIZE];
    
    // 서버 IP 주소 확인
    if(argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        exit(1);
    }
    
    // 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        printf("socket 생성 실패\n");
        exit(1);
    }
    
    // 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(4000);
    
    // 서버 연결
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("connect 실패\n");
        exit(1);
    }
    
    printf("서버에 연결되었습니다.\n");
    
    // 플레이어 번호 수신
    clear_buffer(buff_rcv);
    read(sock, buff_rcv, BUFF_SIZE - 1);
    printf("%s\n", buff_rcv);
    
    while(1) {
        clear_buffer(buff_snd);
        clear_buffer(buff_rcv);
        
        // 사용자 입력 받기
        printf("\n가위/바위/보 중 하나를 입력하세요 (종료: q): ");
        if(fgets(buff_snd, BUFF_SIZE, stdin) != NULL) {
            size_t len = strlen(buff_snd);
            if(len > 0 && buff_snd[len-1] == '\n') {
                buff_snd[len-1] = '\0';
                len--;
            }
            
            // 종료 조건 확인
            if(strcmp(buff_snd, "q") == 0) {
                printf("게임을 종료합니다.\n");
                break;
            }
            
            // 서버로 선택 전송
            write(sock, buff_snd, len);
            
            // 결과 수신 대기
            int read_len = read(sock, buff_rcv, BUFF_SIZE - 1);
            if(read_len <= 0) {
                printf("서버와의 연결이 끊어졌습니다.\n");
                break;
            }
            
            // 결과 출력
            printf("\n%s\n", buff_rcv);
        }
    }
    
    close(sock);
    return 0;
}