#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024 //보낼 데이터, 받을 데이터 버퍼 사이즈

int main(int argc, char **argv){
    int server_socket = 0;
    int client_socket = 0;
    int client_addr_size = 0;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char buff_rcv[BUFF_SIZE+5];
    char buff_snd[BUFF_SIZE+5];

    //구조체 초기화
    memset(&server_addr, 0x00, sizeof(server_addr));
    memset(&client_addr, 0x00, sizeof(client_addr));

    //socket()
    //int socket(int domain, int type, int protocol)
    //domain = PF_INET -> 일반 TCP/IP 인터넷 통신 (AF_UNIX->IPC(프로세스간 통신))
    //type & protocol : SOCK_STREAM & IPPROTO_TCP -> TCP, SOCK_DGRAM & IPPROTO_UDP -> UDP
    //아래 코드에서 0은 기본 프로토콜 타입을 사용하겠다는 의미
    //return값은 socket descripotr이며 0보다 큼, -1은 에러
    server_socket = socket(PF_INET,SOCK_STREAM,0);
    if(-1==server_socket){
        printf("server socket 생성 실패\n");
        exit(1);
    }

    //bind()
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_port = htons(4000); //port num
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //인바운드 IP 지정, 모든 IP가 들어올 수 있음

    if (-1 == bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        printf("bind() error\n");
        exit(1);
    }

    //listen(), listen queue 생성
    if (-1 == listen(server_socket, 5)){ //5개의 client와 연결 가능
        printf("listen() error\n");
        exit(1);
    }

    //accept()
    while(1){
        client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

        if(-1==client_socket){
            printf("클라이언트 연결 실패\n");
            exit(1);
        }

        //read()
        read(client_socket, buff_rcv, BUFF_SIZE);
        printf("receive : %s\n", buff_rcv);

        //write()
        sprintf(buff_snd, "%d : %s", strlen(buff_rcv), buff_rcv);
        write(client_socket, buff_snd, strlen(buff_snd)+1); //+1 : NULL 포함

        //close()
        close(client_socket);
    }

    return 0;
}