#ifndef _UTILS
#define _UTILS

#ifndef USERID
#define USERID int
#endif

#ifndef MSGID
#define MSGID int
#endif

void send_message(USERID, char *);
void send_template_message(char*, char*, char*, USERID);

#endif // _UTILS