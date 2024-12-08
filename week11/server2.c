#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFF_SIZE 1024

void clear_buffer(char *buffer) {
    memset(buffer, 0, BUFF_SIZE);
}

void handle_child(int sig) {
    int status;
    pid_t pid;
    //waitpid의 -1은 모든 자식 프로세스를 대상으로 함
    //WNOHANG을 옵션으로 설정하여 즉시 반환을 보장.
    //이는 함수가 블로킹되지 않고, 종료된 자식 프로세스가 있으면 그 프로세스의 PID를 반환하고, 없으면 0을 반환
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

void game_process(int client_socket, int target_number) {
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];
    
    // 게임 시작 메시지 전송
    sprintf(buff_snd, "숫자 맞추기 게임을 시작합니다! 1-100 사이의 숫자를 맞춰보세요.");
    write(client_socket, buff_snd, strlen(buff_snd));
    
    while(1) {
        clear_buffer(buff_rcv);
        clear_buffer(buff_snd);
        
        // 클라이언트로부터 추측값 수신
        int read_len = read(client_socket, buff_rcv, BUFF_SIZE - 1);
        if(read_len <= 0) {
            printf("Client disconnected\n");
            break;
        }
        
        // 클라이언트가 입력한 숫자
        // <stdlib.h>에 정의되어 있으며 문자열을 정수로 반환
        int guess = atoi(buff_rcv);
        printf("Client guessed: %d (Target: %d)\n", guess, target_number);
        
        if(guess == target_number) {
            sprintf(buff_snd, "정답입니다! 숫자 %d를 맞추셨습니다!", target_number);
            write(client_socket, buff_snd, strlen(buff_snd));
            break;
        } else if(guess < target_number) {
            sprintf(buff_snd, "더 큰 숫자입니다!");
        } else {
            sprintf(buff_snd, "더 작은 숫자입니다!");
        }
        
        write(client_socket, buff_snd, strlen(buff_snd));
    }
    
    close(client_socket);
    exit(0);
}

int main() {
    int server_socket = 0;
    int client_socket = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int client_count = 0;
    
    // SIGCHLD 핸들러 등록
    signal(SIGCHLD, handle_child);
    
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) {
        printf("socket 생성 실패\n");
        exit(1);
    }
    
    // SO_REUSEADDR 옵션 설정
    // 이 옵션은 포트가 사용 중인 상태(TIME_WAIT 상태)일지라도 소켓을 재사용할 수 있도록 커널에 알림
    // 서버를 여러 번 재시작해야 하는 개발 및 테스트 시나리오에서 유용
    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(4000);
    
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("bind 실패\n");
        exit(1);
    }
    
    if(listen(server_socket, 2) == -1) {
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
        
        client_count++;
        printf("New client connected: %s (Client #%d)\n", inet_ntoa(client_addr.sin_addr), client_count);
        
        pid_t pid = fork();
        
        if(pid == -1) {
            printf("fork 실패\n");
            close(client_socket);
            continue;
        }
        else if(pid == 0) {  // 자식 프로세스
            close(server_socket);  // 자식 프로세스는 서버 소켓 필요 없음
            
            // 클라이언트 번호에 따라 다른 타겟 넘버 설정
            int target_number = (client_count == 1) ? 10 : 20;
            
            game_process(client_socket, target_number);
            return 0;
        }
        else {  // 부모 프로세스
            close(client_socket);  // 부모 프로세스는 클라이언트 소켓 필요 없음
        }
    }
    
    close(server_socket);
    return 0;
}