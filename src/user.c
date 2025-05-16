#include "../includes/chat.h"

// INTERNAL
USERID get_id() {
    for (int i = 0; i < ROOMSIZE; i++) {
        if (users[i]->type == INACTIVE) {
            return i;
        }
    }
    return FULL_ROOM;
}

/*
 * gets a user by userid
 */
User* get_user(USERID userid) {
    pthread_mutex_lock(&lock);
    if (userid < ROOMSIZE) {
         pthread_mutex_unlock(&lock);
        return users[userid];
    }
    pthread_mutex_unlock(&lock);
    return NULL;
} /* get_user() */

/*
 * Add user with specified name and fd
 */
USERID join(int fd, char* name) {
    if (user_count >= ROOMSIZE) {
        return FULL_ROOM;
    }
    pthread_mutex_lock(&lock);
    USERID userid = get_id();
    if (userid == FULL_ROOM) {
        pthread_mutex_unlock(&lock);
        return FULL_ROOM;
    }

    User* new_user = users[userid];
    new_user->name = name;
    new_user->style = 0;
    new_user->fd = fd;
    new_user->type = ACTIVE;

    user_count++;
    pthread_mutex_unlock(&lock);
    return userid;
} /* join() */

/*
 * call when a user with userid leaves the room
 * frees the user's username, and closes the user's fd
 */
void leave(USERID userid) {
    if (userid >= ROOMSIZE) {
        return;
    }
    pthread_mutex_lock(&lock);
    User* leaving_user = users[userid];
    if (leaving_user->type == ACTIVE) {
        leaving_user->type = INACTIVE;
        free(leaving_user->name);
        leaving_user->name = NULL;
        close(leaving_user->fd);
        if (user_count > 0) {
            user_count--;
        }
    }
    pthread_mutex_unlock(&lock);
} /* leave() */