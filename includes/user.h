#ifndef _USER
#define _USER

#define USERID int
#define MSGID int

#define ROOMSIZE (100)
#define FULL_ROOM (-1)

typedef enum user_type {
    INACTIVE,
    ACTIVE
} UserType;

typedef struct user {
    int fd;
    char* name;
    int style;
    UserType type;
} User;

User* get_user(USERID);
USERID join(int, char *);
void leave(USERID);
void send_message(USERID, char *);

#endif // _USER