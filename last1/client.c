#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>


typedef struct{
    char name[10];
    int age;
    char message[100];
} Chat; 

int main(int argc, char* argv[]) {
    int server_sock;
    struct sockaddr_in server_addr;
    Chat chat;
    char buff[1024];

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    connect(server_sock,(struct sockaddr*)&server_addr, sizeof(server_addr));

    while(1){
        memset(&buff,0,sizeof(buff));
        memset(&chat,0,sizeof(Chat));

        printf("입력하세요.(이름:나이 > 메시지)\n");

        scanf("%[^:]:%d > %[^\n]", chat.name, &chat.age, chat.message);
        write(server_sock, &chat, sizeof(chat));

        printf("write success\n");
        printf("name : %s,  age : %d, message : %s \n", chat.name, chat.age, chat.message);

        int read_len = read(server_sock, buff, sizeof(buff));
        printf("received : %s\n", buff);

        if(strncmp(buff,"bye",3)==0){
            printf("disconnect\n");
            close(server_sock);
            break;
        }
    }
    
    return 0;
}