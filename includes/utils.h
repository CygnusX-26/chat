#ifndef _UTILS
#define _UTILS

#include "user.h"

void send_message(USERID, char *);
char* template_message(const char*, const char*, const char*);
char* template_message2(const char*, const char*, const char*, const char*);
void send_template_message(const char*, const char*, const char*, USERID);
void send_template_message2(const char*, const char*, const char*, const char*, USERID);
User** get_active_users(int*);
char* comma_separated_active_users();

#endif // _UTILS