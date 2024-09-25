#ifndef VOTE_H
#define VOTE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mysql.h>
#include <hiredis.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>

#define PORT 8081
#define MAX_OPTIONS 10
#define MAX_NAME_LEN 50
#define BUFFER_SIZE 1024
extern const char *hostname;
extern const char *username;
extern const char *password; // MySQL密码
extern const char *dbname; // 要连接的数据库名
extern redisContext *redis_ctx;
extern MYSQL *mysql_conn;

typedef struct {
    int writer_id;
    int score;
} Vote;


// 函数声明
void init_db_connections();

void close_db_connections();

int add_vote(int writer_id);

int delete_vote(int writer_id, const char *passwd);

int update_vote(int writer_id, int score);

int get_vote_score(int writer_id);

void handle_client(int sockfd);

void run_server();

void run_client();

void create_vote(char *writer_name, char *passwd);

void toLowerCase(char *str);

void init_mysql();

void init_redis();

void top_redis(redisContext *redis_ctx, int client_socket);

int send_data_to_client(int client_socket, const char *data, size_t length);

void top_vote(redisContext *redis_ctx, int client_socket);

int create_mysql(char *writer_name, char *passwd);

void create_redis(int writer_id);

void add_mysql(int writer_id);

void add_redis(int writer_id);

int update_mysql(int writer_id, int new_score);

int update_redis(int writer_id, int new_score);

int get_vote_score_redis(int writer_id);

int delete_mysql(int writer_id, const char *passwd);

int delete_redis(int writer_id);

#endif // VOTE_H