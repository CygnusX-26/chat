#ifndef _CHAT
#define _CHAT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>


#include "user.h"
#include "utils.h"

#define PORT (34446)
#define BACKLOG (5)
#define MESSAGE_LEN (1024)
#define USERNAME_LEN (20)
#define SYSTEM (-1)

extern User** users;
extern int user_count;
extern pthread_mutex_t lock;

#endif // _CHAT