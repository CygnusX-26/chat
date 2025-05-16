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

/*
 * sends a templated message  with pretext, contents, and posttext formatted.
 */
void send_template_message(char* text_pre, char* contents, char* text_post, USERID userid) {
    char* message = malloc(sizeof(char) * (strlen(contents) + strlen(text_pre) + strlen(text_post) + 1));
    strcpy(message, text_pre);
    strcat(message, contents);
    strcat(message, text_post);
    send_message(userid, message);
    free(message);
    message = NULL;
} /* send_template_message() */