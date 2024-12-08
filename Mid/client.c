

int main(int argc, char** argv) {


    while(1) {
        scanf("%s", buff);
        ret = write(clnt_sock, buff, strlen(buff) + 1);
        if(ret < 0) {
            printf("write error\n");
            break;
        }
        if(strlen(buff) == 1 && buff[0] == 'q') {
            printf("클라이언트 종료 요청\n");
            break;
        }
        memset(buff, 0x00, sizeof(buff));
        ret = read(clnt_sock, buff, BUFF_SIZE);
        if(ret < 0) {
            printf("read error\n");
            break;
        }
        printf("%s\n", buff);
    }
    sleep(1); // 던지자 마자 끊어버림. sleep으로 패킷 도착할 때까지 약간 여유를 줌.
    close(clnt_sock);
}
