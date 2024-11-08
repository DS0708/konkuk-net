#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024 //보낼 데이터, 받을 데이터 버퍼 사이즈

int main(int argc, char **argv){
    
    int client_socket;

    struct sockaddr_in server_addr;

    char buff[BUFF_SIZE+5]; //1029byte

    //socket()
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == client_socket){
        printf("Clinet Socket 생성 실패");
        exit(1);
    }

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000); //server port number
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //connect()
    if(-1==connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        printf("접속 실패\n");
        exit(1);
    }

    //write()/read()
    write(client_socket, argv[1], strlen(argv[1])+1); //+1 : null
    read(client_socket, &buff, BUFF_SIZE);
    printf("receive : %s\n", buff);
    close(client_socket);
    


    return 0;
}