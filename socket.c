int server_socket;
server_socket = socket(PF_INET, SOCK_STREAM, 0);
if(-1 == server_socket)
{
printf("server socket 생성 실패\n“);
exit(1);
}