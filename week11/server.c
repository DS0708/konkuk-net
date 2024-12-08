//가위바위보 게임 제작
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
#define PLAYER_COUNT 2

typedef struct {
    int socket;
    int player_num;
} PlayerInfo;

// 게임 결과를 저장할 공유 메모리 구조체
typedef struct {
    char choices[2][10];  // 각 플레이어의 선택
    int ready_count;      // 준비된 플레이어 수
} GameState;

void clear_buffer(char *buffer) {
    memset(buffer, 0, BUFF_SIZE);
}

// 가위바위보 승자 판정
int determine_winner(const char* p1_choice, const char* p2_choice) {
    if (strcmp(p1_choice, p2_choice) == 0) return 0;  // 비김
    
    if ((strcmp(p1_choice, "가위") == 0 && strcmp(p2_choice, "보") == 0) ||
        (strcmp(p1_choice, "바위") == 0 && strcmp(p2_choice, "가위") == 0) ||
        (strcmp(p1_choice, "보") == 0 && strcmp(p2_choice, "바위") == 0)) {
        return 1;  // 플레이어 1 승
    }
    
    return 2;  // 플레이어 2 승
}

// 클라이언트 처리 프로세스
void handle_client(PlayerInfo player_info, int* shared_pipe) {
    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];
    
    // 플레이어 번호 알림
    snprintf(buff_snd, BUFF_SIZE, "당신은 플레이어 %d입니다. '가위', '바위', '보' 중 하나를 입력하세요.", player_info.player_num);
    write(player_info.socket, buff_snd, strlen(buff_snd));
    
    while(1) {
        clear_buffer(buff_rcv);
        clear_buffer(buff_snd);
        
        // 클라이언트로부터 선택 받기
        int read_len = read(player_info.socket, buff_rcv, BUFF_SIZE - 1);
        if(read_len <= 0) {
            printf("Player %d disconnected\n", player_info.player_num);
            break;
        }
        
        // 입력값 검증
        buff_rcv[strcspn(buff_rcv, "\n")] = 0;  // 개행 제거
        if(strcmp(buff_rcv, "가위") != 0 && 
           strcmp(buff_rcv, "바위") != 0 && 
           strcmp(buff_rcv, "보") != 0) {
            strcpy(buff_snd, "잘못된 입력입니다. '가위', '바위', '보' 중 하나를 입력하세요.");
            write(player_info.socket, buff_snd, strlen(buff_snd));
            continue;
        }
        
        // 파이프에 선택 전송
        write(shared_pipe[1], buff_rcv, strlen(buff_rcv) + 1);
        
        // 결과 대기
        clear_buffer(buff_rcv);
        read(shared_pipe[0], buff_rcv, BUFF_SIZE);
        
        // 결과 전송
        write(player_info.socket, buff_rcv, strlen(buff_rcv));
    }
    
    close(player_info.socket);
    exit(0);
}

int main() {
    int server_socket = 0;
    int client_socket = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int shared_pipe[2];  // 프로세스 간 통신을 위한 파이프
    
    // 파이프 생성
    if(pipe(shared_pipe) == -1) {
        printf("파이프 생성 실패\n");
        exit(1);
    }
    
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
    
    printf("가위바위보 게임 서버 시작. 클라이언트 대기 중...\n");
    
    while(1) {
        GameState game_state = {0};
        pid_t child_pids[PLAYER_COUNT] = {0};
        
        // 두 플레이어 접속 대기
        for(int i = 0; i < PLAYER_COUNT; i++) {
            socklen_t client_addr_size = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
            
            if(client_socket == -1) {
                printf("accept 실패\n");
                continue;
            }
            
            printf("Player %d connected: %s\n", i+1, inet_ntoa(client_addr.sin_addr));
            
            // 자식 프로세스 생성
            PlayerInfo player_info = {client_socket, i+1};
            pid_t pid = fork();
            
            if(pid == 0) {  // 자식 프로세스
                handle_client(player_info, shared_pipe);
            } else {  // 부모 프로세스
                child_pids[i] = pid;
            }
        }
        
        // 부모 프로세스: 게임 진행 관리
        char choices[PLAYER_COUNT][BUFF_SIZE] = {0};
        int ready_count = 0;
        
        while(1) {
            char buff[BUFF_SIZE] = {0};
            read(shared_pipe[0], buff, BUFF_SIZE);
            
            strcpy(choices[ready_count], buff);
            ready_count++;
            
            if(ready_count == PLAYER_COUNT) {
                // 승자 판정
                int winner = determine_winner(choices[0], choices[1]);
                char result[BUFF_SIZE];
                
                if(winner == 0) {
                    sprintf(result, "비겼습니다! (P1: %s, P2: %s)", choices[0], choices[1]);
                } else {
                    sprintf(result, "플레이어 %d가 이겼습니다! (P1: %s, P2: %s)", 
                            winner, choices[0], choices[1]);
                }
                
                // 결과를 양쪽 모두에게 전송
                write(shared_pipe[1], result, strlen(result) + 1);
                write(shared_pipe[1], result, strlen(result) + 1);
                
                ready_count = 0;
                memset(choices, 0, sizeof(choices));
            }
        }
    }
    
    close(server_socket);
    return 0;
}