#include "public.h"

extern MYSQL *mysql_conn;
const char *hostname = "localhost";
const char *username = "ecs-user";
const char *password = "ecs-user"; // MySQL密码
const char *dbname = "Vote"; // 要连接的数据库名
const unsigned int port = 3306; // MySQL服务器的端口，默认为3306
void init_mysql() {
    mysql_conn = mysql_init(NULL);
    if (mysql_conn == NULL) {
        fprintf(stderr, "MySQL init failed\n");
        exit(EXIT_FAILURE);
    }
    if (!mysql_real_connect(mysql_conn, hostname, username, password, dbname, 0, NULL, 0)) {
        fprintf(stderr, "MySQL connect error: %s\n", mysql_error(mysql_conn));
        exit(EXIT_FAILURE);
    }
}

int create_mysql(char *writer_name, char *passwd) {
    char query_writers[256];
    sprintf(query_writers, "INSERT INTO writers (writer_name, score, passwd) VALUES ('%s', 10, '%s')", writer_name,
            passwd);

    if (mysql_query(mysql_conn, query_writers)) {
        fprintf(stderr, "MySQL add writers error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    // 查询刚插入的记录的 writer_id
    sprintf(query_writers, "SELECT writer_id FROM writers WHERE writer_name = '%s'", writer_name);
    if (mysql_query(mysql_conn, query_writers)) {
        fprintf(stderr, "MySQL query writer_id error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(mysql_conn);
    if (result == NULL) {
        fprintf(stderr, "MySQL store result error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {
        fprintf(stderr, "No such writer_name: %s\n", writer_name);
        mysql_free_result(result);
        return -1;
    }

    int writer_id = atoi(row[0]);
    char post_query[256];
    sprintf(post_query, "INSERT INTO posts (writer_id,content) VALUES ('%d','hello')", writer_id);

    if (mysql_query(mysql_conn, post_query)) {
        fprintf(stderr, "MySQL insert post error: %s\n", mysql_error(mysql_conn));
        return -1;
    }
    sprintf(post_query, "SELECT post_id FROM posts WHERE writer_id = '%d'", writer_id);
    if (mysql_query(mysql_conn, post_query)) {
        fprintf(stderr, "MySQL query writer_id error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    MYSQL_RES *rresult = mysql_store_result(mysql_conn);
    if (rresult == NULL) {
        fprintf(stderr, "MySQL store result error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    MYSQL_ROW post_id = mysql_fetch_row(rresult);
    if (post_id == NULL) {
        fprintf(stderr, "No such writer_name: %s\n", writer_name);
        mysql_free_result(result);
        return -1;
    }
    int ppost_id = atoi(post_id[0]);
    char PostMetadata_query[256];
    sprintf(PostMetadata_query,
            "INSERT INTO PostMetadata (post_id,title,post_type,status) VALUES ('%d','sport','text','published')",
            ppost_id);

    if (mysql_query(mysql_conn, PostMetadata_query)) {
        fprintf(stderr, "MySQL insert post error: %s\n", mysql_error(mysql_conn));
        return -1;
    }
    mysql_free_result(result);
    return writer_id;
}

void add_mysql(int writer_id) {
    char query[256];
    sprintf(query, "INSERT INTO writers (writer_id, score) VALUES (%d, 10) ON DUPLICATE KEY UPDATE score=score+10",
            writer_id);

    // 执行 MySQL 查询
    if (mysql_query(mysql_conn, query)) {
        fprintf(stderr, "MySQL add vote error: %s\n", mysql_error(mysql_conn));
        return;
    }
}

int update_mysql(int writer_id, int new_score) {
    char query[256];
    sprintf(query, "UPDATE writers SET score = %d WHERE writer_id = %d", new_score, writer_id);
    if (mysql_query(mysql_conn, query)) {
        fprintf(stderr, "MySQL update vote error: %s\n", mysql_error(mysql_conn));
        return -1;
    }
}

int delete_mysql(int writer_id, const char *passwd) {
    char query[256];
    sprintf(query, "SELECT COUNT(*) FROM writers WHERE writer_id = %d AND passwd = '%s'", writer_id, passwd);

    if (mysql_query(mysql_conn, query)) {
        fprintf(stderr, "MySQL query error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(mysql_conn);
    if (result == NULL) {
        fprintf(stderr, "MySQL store result error: %s\n", mysql_error(mysql_conn));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL || atoi(row[0]) == 0) {
        // 密码错误或没有找到对应的 writer_id
        fprintf(stderr, "Invalid password or writer_id not found: %d\n", writer_id);
        mysql_free_result(result);
        return -1;
    }

    // 密码正确，执行删除操作
    sprintf(query, "DELETE FROM votes WHERE writer_id = %d", writer_id);
    if (mysql_query(mysql_conn, query)) {
        fprintf(stderr, "MySQL delete vote error: %s\n", mysql_error(mysql_conn));
        mysql_free_result(result);
        return -1;
    }
    mysql_free_result(result);
}