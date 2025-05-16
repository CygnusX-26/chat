#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#include "../includes/chat.h"

User** users;
int user_count;
pthread_mutex_t lock;

const char* colors[] = {
    "\033[94m", "\033[92m", "\033[96m",
    "\033[95m", "\033[93m", "\033[91m"
};

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

    char* active = comma_separated_active_users();
    char* tmsg = template_message("Welcome to the scuffed chat room...\r\n"
        "\033[96m[~] Currently online: ", active, "\033[0m\r\n"
        "name: "
    );
    send(client_fd, tmsg, strlen(tmsg), 0);
    free(active);
    free(tmsg);
    tmsg = NULL;
    active = NULL;

    char* username = calloc(USERNAME_LEN + 1, sizeof(char));
    if (recv(client_fd, username, USERNAME_LEN, 0) <= 0) {
        free(username);
        username = NULL;
        return NULL;
    }

    char *p = strchr(username, '\n');
    if (p) {
        *p = '\0';
    }

    USERID userid = join(client_fd, username);

    send_template_message("\033[92m[+] ", username, " has joined.\033[0m\r\n", SYSTEM);

    char buffer[MESSAGE_LEN];
    ssize_t bytes_read = 0;
    int color_index = (time(NULL)) % 6;
    while ((bytes_read = recv(client_fd, buffer, MESSAGE_LEN, 0)) > 0) {
        send_template_message2(colors[color_index], username, "\033[0m: ", buffer, userid);
        memset(buffer, 0, MESSAGE_LEN);
    }

    send_template_message("\033[91m[-] ", username, " has left.\033[0m\r\n", SYSTEM);

    leave(userid);
    return NULL;
}

int main() {

    init();

    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("mutex init");
        exit(EXIT_FAILURE);
    }

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
        fflush(stdout);

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