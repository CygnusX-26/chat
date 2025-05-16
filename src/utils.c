#include "../includes/chat.h"

/*
 * sends a message specified in message.
 * Message will need to be freed if malloc'ed.
 */
void send_message(USERID sender, char* message) {
    if (message == NULL) {
        return;
    }
    pthread_mutex_lock(&lock);
    for (int i = 0; i < ROOMSIZE; i++) {
        if (users[i]->type == ACTIVE && i != sender) {
            send(users[i]->fd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&lock);
} /* send_message() */

char* template_message(const char* text_pre, const char* contents, const char* text_post) {
    char* message = malloc(sizeof(char) *
        (strlen(contents) + strlen(text_pre) + strlen(text_post) + 1));
    strcpy(message, text_pre);
    strcat(message, contents);
    strcat(message, text_post);
    return message;
}

/*
 * sends a templated message  with pretext, contents,
 * and posttext formatted.
 */
void send_template_message(const char* text_pre, const char* contents, const char* text_post, USERID userid) {
    char* message = template_message(text_pre, contents, text_post);
    send_message(userid, message);
    free(message);
    message = NULL;
} /* send_template_message() */

char* template_message2(const char* text_pre, const char* content1, const char* content2, const char* text_post) {
    char* message = malloc(sizeof(char) *
        (strlen(content1) + strlen(content2) + strlen(text_pre) + strlen(text_post) + 1));
    strcpy(message, text_pre);
    strcat(message, content1);
    strcat(message, content2);
    strcat(message, text_post);
    return message;
}

/*
 * sends a templated message with pretext, contents, contents2
 * and posttext formatted.
 */
void send_template_message2(const char* text_pre, const char* content1, const char* content2, const char* text_post, USERID userid) {
    char* message = template_message2(text_pre, content1, content2, text_post);
    send_message(userid, message);
    free(message);
    message = NULL;
} /* send_template_message2() */

/*
 * returns a list of pointers to all active users.
 * The list is malloced and will need to be freed.
 */
User** get_active_users(int* num_users) {
    pthread_mutex_lock(&lock);
    User** active_users = NULL;
    int c = 0;
    for (int i = 0; i< ROOMSIZE; i++) {
        if (users[i]->type == ACTIVE) {
            active_users = realloc(active_users, sizeof(User*) * (c + 1));
            active_users[c++] = users[i];
        }
    }
    *num_users = c;
    pthread_mutex_unlock(&lock);
    return active_users;
} /* get_active_users() */

char* comma_separated_active_users() {
    int num_active_users = 0;
    User** active_users = get_active_users(&num_active_users);
    char* user_string = NULL;
    int string_length = 0;
    char* name = NULL;

    if (num_active_users == 0) {
        user_string = calloc(5, sizeof(char));
        strcpy(user_string, "None");
    }
    else {
        for (int i = 0; i < num_active_users; i++) {
            name = active_users[i]->name;
            user_string = realloc(user_string, sizeof(char) * (string_length + strlen(name) + 3));
            strcat(user_string, name);
            if (i != num_active_users - 1) {
                strcat(user_string, ", ");
            }
            string_length += strlen(user_string);
        }
        free(active_users);
        active_users = NULL;
    }
    
    return user_string;
}