#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sqlite3* db;

/*
 * initializes the db using db/init.sql
 */
void init_db() {
    if (sqlite3_open("db/history.db", &db) != 0) {
        perror("sqlite3_open");
        exit(EXIT_FAILURE);
    }

    FILE* fp = fopen("db/init.sql", "r");

    if (!fp) {
        perror("fopen init.sql");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *sql = malloc(len + 1);
    fread(sql, 1, len, fp);
    sql[len] = '\0';
    fclose(fp);

    char* err_msg = NULL;

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    free(sql);
    sql = NULL;
} /* init_db() */

//INTERNAL
int handle_recent(void* data, int argc, char** argv, char** colNames) {
    (void) argc;
    (void) colNames;

    char** messages = (char**) data;

    const char *message = argv[0] ? argv[0] : "";
    *messages = realloc(*messages, sizeof(char) * (strlen(*messages) + strlen(message) + 1));
    strcat(*messages, message);
    return 0;
}

/*
 * gets the num most recent messages and returns them as a malloc'ed string.
 */
char* get_recent_messages(int num) {
    (void) num; //TODO not make this hardcoded
    char* sql = "SELECT * FROM "
        "(SELECT message, created_at FROM chat_history "
        "ORDER BY created_at DESC LIMIT 10) "
        "ORDER BY created_at ASC;";

    char* messages = calloc(1, sizeof(char));
    char* err_msg = NULL;

    if (sqlite3_exec(db, sql, handle_recent, &messages, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    return messages;
} /* get_recent_messages() */

/*
 * inserts a message into the message history
 */
void insert_message(char* message) {
    char* sql = "INSERT INTO chat_history (message) VALUES (?);";
    sqlite3_stmt* stmt = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_bind_text(stmt, 1, message, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind message: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
} /* insert_message */
