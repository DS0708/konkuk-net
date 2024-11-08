#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

int main(){

    int c_sock, s_sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int client_addr_size;

    memset(&server_addr,0x00,sizeof(server_addr));
    memset(&client_addr,0x00,sizeof(client_addr));

    char buff_snd[BUFF_SIZE];
    char buff_rcv[BUFF_SIZE];

    s_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(s_sock == -1){
        printf("socket error\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(s_sock,(struct sockaddr *)&server_addr, sizeof(server_addr))){
        printf("socket error\n");
        exit(1);
    }

    if(listen(s_sock, 5) == -1){
        printf("socket error\n");
        exit(1);
    }

    while(1){
        client_addr_size = sizeof(client_addr);
        c_sock = accept(s_sock, (struct sockaddr *)&client_addr, client_addr_size);

        if(c_sock == -1){
            printf("socket error\n");
            break;
        }

        while(1){
            memset(buff_rcv,0,BUFF_SIZE);
            memset(buff_snd,0,BUFF_SIZE);
            //read
            int ret = read(c_sock, buff_rcv, BUFF_SIZE);
            if(ret <=0){
                printf("socket error\n");
                break;
            }
            if(strlen(buff_rcv)==1 && buff_rcv[0]=='q'){
                printf("socket quit\n");
                break;
            }

            printf("clinet : %s\n", buff_rcv);
            //write
            if(fgets(buff_snd,BUFF_SIZE,stdin)!=NULL){
                int len = strlen(buff_snd);
                if(len > 0 && buff_snd[len-1]=='\n'){
                    buff_snd[len-1] = '\0';
                    len--;
                }


            }
        }

        close(c_sock);
    }

    close(s_sock)
    return 0;
}