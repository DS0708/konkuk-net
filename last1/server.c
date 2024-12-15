#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct{
    char name[10];
    int age;
    char message[100];
} Chat; 

int main(int argc, char* argv[]) {
    int client_sock, server_sock;
    struct sockaddr_in server_addr;
    char buff[1024];
    fd_set readfds;
    int max_fds = 2;
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);
    
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    listen(server_sock, 5);

    while(1){
        struct sockaddr_in client_addr;
        int client_addr_len;
        client_sock = accept(server_sock,(struct sockaddr*)&client_addr, &client_addr_len);
        Chat chat;
        FD_ZERO(&readfds);
        FD_SET(client_sock, &readfds);
        max_fds = client_sock;
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        printf("connection success\n");
        
        while(1){
            memset(&buff,0,sizeof(buff));
            memset(&chat,0,sizeof(Chat));

            int state = select(max_fds+1, &readfds, 0, 0, &timeout);

            if(state < 0){
                printf("select error\n");
            }else if (state == 0){
                printf("client timeout\n");
                write(client_sock,"bye\0",3);
                close(client_sock);
                break;
            }else if (FD_ISSET(client_sock, &readfds)){
                
                int read_len = read(client_sock, &chat, sizeof(Chat));

                printf("name : %s,  age : %d, message : %s \n", chat.name, chat.age, chat.message);
                printf("입력:");

                memset(&buff,0,sizeof(buff));
                if(fgets(buff,sizeof(buff),stdin) != NULL){
                    buff[strlen(buff)-1] = '\0';
                    write(client_sock, buff, sizeof(buff));
                }

                if(strncmp(buff,"bye",3)==0){
                    printf("disconnect\n");
                    close(client_sock);
                    break;
                }
            }
        }
        
    }
    

    return 0;
}