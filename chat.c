#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "includes/chat.h"
#include "includes/user.h"

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

    char* join_text_pre = "** ";
    char* join_text_post = " has joined ** \n";
    char* join_message = malloc(sizeof(char) * (username_len + strlen(join_text_post) + strlen(join_text_pre) + 1));
    strcpy(join_message, join_text_pre);
    strcat(join_message, username);
    strcat(join_message, join_text_post);
    send_message(SYSTEM, join_message);
    free(join_message);
    join_message = NULL;

    char* message_pre = "";
    char* message_post = ": ";
    char* message = malloc(username_len + MESSAGE_LEN + strlen(message_pre) + strlen(message_post) + 1);
    strcpy(message, message_pre);
    strcat(message, user->name);
    strcat(message, message_post);

    char* buffer = message + username_len + 2;
    ssize_t bytes_read = 0;
    while ((bytes_read = recv(client_fd, message + username_len + 2, MESSAGE_LEN, 0)) > 0) {
        send_message(userid, message);
        memset(buffer, 0, MESSAGE_LEN);
    }
    
    free(message);
    message = NULL;

    char* leave_text_pre = "** ";
    char* leave_text_post = " has left ** \n";
    char* leave_message = malloc(sizeof(char) * (username_len + strlen(leave_text_pre) + strlen(leave_text_post) + 1));
    strcpy(leave_message, leave_text_pre);
    strcat(leave_message, username);
    strcat(leave_message, leave_text_post);
    send_message(SYSTEM, leave_message);
    free(leave_message);
    leave_message = NULL;

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