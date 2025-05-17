#ifndef _DB
#define _DB

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_db();
char* get_recent_messages(int);
void insert_message(char*);

#endif // _DB