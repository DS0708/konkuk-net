#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    int id;
    int is_active;
} client_info;

client_info clients[MAX_CLIENTS] = {0};
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

client_info* add_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_active == 0) {
            clients[i].is_active = 1;
            clients[i].socket = client_socket;
            clients[i].id = i + 1;
            client_count++;
            pthread_mutex_unlock(&clients_mutex);
            return &clients[i];
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return NULL;
}

void remove_client(int client_id) {
    pthread_mutex_lock(&clients_mutex);
    if (clients[client_id-1].is_active == 1) {
        close(clients[client_id-1].socket);
        clients[client_id-1].is_active = 0;
        client_count--;
    }else{
        printf("Remove Error: Client ID %d not found.\n", client_id);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(const char* message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_active == 1) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_all_client(){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_active == 1) {
            close(clients[i].socket);
            clients[i].is_active = 0;
            client_count--;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_client(int client_id, const char* message) {
    int found = 0;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_active == 1 && clients[i].id == client_id) {
            send(clients[i].socket, message, strlen(message), 0);
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    if (!found) {
        printf("Send Error: Client ID %d not found.\n", client_id);
    }
}

void* handle_client(void* arg) {
    client_info* client = (client_info*)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(client->socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';

        pthread_mutex_lock(&print_mutex);
        printf("Received from client %d: %s", client->id, buffer);
        pthread_mutex_unlock(&print_mutex);

        if (strcmp(buffer, "bye\n") == 0) {
            printf("Connection with client %d has been disconnected by the client.\n\n", client->id);
            remove_client(client->id);
            break;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;
    fd_set readfds;
    char server_cmd[BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed\n");
        exit(1);
    }

    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed\n");
        exit(1);
    }

    if(listen(server_socket, MAX_CLIENTS) == -1) {
        printf("Listen failed\n");
        exit(1);
    }

    printf("Server listening on port 8888...\n");
    printf("Enter server command (to<clientID> <message> / all <message> / bye <clientID> / allbye):\n\n");

    while(1){
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(server_socket, &readfds);
        int max_fd = server_socket;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error\n");
            continue;
        }
        //Client Logic
        if (FD_ISSET(server_socket, &readfds)) {
            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_socket < 0) {
                perror("Accept failed\n");
                continue;
            }

            client_info* new_client = add_client(client_socket);
            if (new_client == NULL) {
                perror("Failed to add client\n");
                continue;
            }

            printf("new client id : %d\n\n", new_client->id);

            if (pthread_create(&thread_id, NULL, handle_client, (void*)new_client) != 0) {
                perror("Thread creation failed\n");
                remove_client(new_client->id);
            }
            pthread_detach(thread_id);
        }
        //Server Logic
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(server_cmd, BUFFER_SIZE, stdin);
            printf("Server command processed: %s\n", server_cmd);

            if (strncmp(server_cmd, "to", 2) == 0) {
                int target_id = atoi(server_cmd + 2);
                char* message = strchr(server_cmd, ' ');
                if (message) {
                    send_to_client(target_id, message + 1);
                    if(strncmp(message, " bye",4) == 0) {
                        remove_client(target_id);
                        printf("Connection with client %d has been disconnected by the server.\n\n", target_id);
                    }
                }
            }
            else if (strcmp(server_cmd, "allbye\n") == 0) {
                broadcast_message("all bye\n");
                remove_all_client();
            }
            else if (strncmp(server_cmd, "all", 3) == 0) {
                broadcast_message(server_cmd + 4);
                char* message = strchr(server_cmd, ' ');
                if(strncmp(message, " bye",4) == 0) {
                    remove_all_client();
                }
            }
        }
    }

    close(server_socket);
}