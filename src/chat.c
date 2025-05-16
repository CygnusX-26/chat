#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../includes/chat.h"
#include "../includes/user.h"

User** users;
int user_count;
pthread_mutex_t lock;

void init() {
    users = calloc(ROOMSIZE, sizeof(User*));
    for (int i = 0; i < ROOMSIZE; i++) {
        User* empty = malloc(sizeof(User));
        empty->type = INACTIVE;
        users[i] = empty;
    }
    user_count = 0;
}

void* handle_client(void* fd) {
    int client_fd = *(int* )fd;
    free(fd);
    fd = NULL;

    char* prompt = "Welcome to the scuffed chat room...\nusername: ";
    send(client_fd, prompt, strlen(prompt), 0);

    char* username = malloc(sizeof(char) * (USERNAME_LEN + 1));
    if (recv(client_fd, username, USERNAME_LEN, 0) < 0) {
        free(username);
        username = NULL;
        return NULL;
    }

    char *p = strchr(username, '\n');
    if (p) {
        *p = '\0';
    }

    USERID userid = join(client_fd, username);
    User* user = get_user(userid);
    ssize_t username_len = strlen(user->name);

    send_template_message("** ", username, " has joined **\n", SYSTEM);

    char buffer[MESSAGE_LEN];
    ssize_t bytes_read = 0;
    while ((bytes_read = recv(client_fd, buffer, MESSAGE_LEN, 0)) > 0) {
        send_template_message(username, ": ", buffer, userid);
        memset(buffer, 0, MESSAGE_LEN);
    }

    send_template_message("** ", username, " has left **\n", SYSTEM);

    leave(userid);
    return NULL;
}

int main(int argc, char** argv) {

    init();

    int server_fd = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    struct sockaddr_in addr_in;

    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = INADDR_ANY;
    addr_in.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr* )&addr_in, sizeof(addr_in)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d\n", PORT);
    fflush(stdout);

    int* client_fd;
    int addr_in_len = sizeof(addr_in);

    while (1) {
        client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr* )&addr_in, (socklen_t*)&addr_in_len);
        if (*client_fd < 0) {
            perror("accept");
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(addr_in.sin_addr), ntohs(addr_in.sin_port));

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(tid);
        }
    }
}