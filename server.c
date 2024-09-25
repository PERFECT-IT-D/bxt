
#include "public.h"

MYSQL *mysql_conn;
redisContext *redis_ctx;

void init_db_connections() {
    init_mysql();
    init_redis();
}

void close_db_connections() {
    mysql_close(mysql_conn);
    redisFree(redis_ctx);
}

void top_vote(redisContext *redis_ctx, int client_socket) {
    top_redis(redis_ctx, client_socket);
}

void create_vote(char *writer_name, char *passwd) {
    //了解下如何防止SQL注入攻击
    // 准备 SQL 语句插入投票记录
    int writer_id = create_mysql(writer_name, passwd);
    create_redis(writer_id);
    // 构建 Redis 命令
}

int add_vote(int writer_id) {
    // 构建 MySQL 查询
    add_mysql(writer_id);
    add_redis(writer_id);
    // 构建 Redis 命令
    return 0;
}


int update_vote(int writer_id, int new_score) {
    update_mysql(writer_id, new_score);
    update_redis(writer_id, new_score);
    return 0;
}

int get_vote_score(int writer_id) {
    // 构建 Redis 命令字符串
    get_vote_score_redis(writer_id);

}

int delete_vote(int writer_id, const char *passwd) {
    if (delete_mysql(writer_id, passwd) == -1) {
        printf("passwd is not correct\n");
        return -1;
    }
    if (delete_redis(writer_id) == -1) {
        printf("redis delete error\n");
    }
    return 0;
    // 构建 Redis 删除命令

}

void toLowerCase(char *str) {
    if (str) {
        char *ptr = str;
        while (*ptr) {
            *ptr = tolower((unsigned char) *ptr); // 转换为小写
            ptr++;
        }
    }
}

void handle_client(int sockfd) {
    char buffer[1024] = {0};
    char reply_msg[256] = {0};
    char *saveptr = NULL; // 用于保存 strtok_r 的状态

    // 读取客户端发送的数据
    int read_size = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (read_size <= 0) {
        perror("Read failed");
        return;
    }
    buffer[read_size] = '\0';

    // 使用 strtok_r 解析操作命令
    char *operation = strtok_r(buffer, " ", &saveptr);
    if (operation == NULL) {
        sprintf(reply_msg, "EMPTY COMMAND\n");
        send(sockfd, reply_msg, strlen(reply_msg), 0);
        return;
    }

    // 转换操作命令为小写
    toLowerCase(operation);

    // 定义操作类型枚举
    enum {
        TOP,
        UPDATE,
        DELETE_OP,
        CREATE,
        ADD,
        GET,
    } operation_type;

    // 根据操作命令的首字符设置操作类型
    switch (operation[0]) {
        case 't':
            operation_type = TOP;
            break;
        case 'u':
            operation_type = UPDATE;
            break;
        case 'd':
            operation_type = DELETE_OP;
            break;
        case 'c':
            operation_type = CREATE;
            break;
        case 'a':
            operation_type = ADD;
            break;
        case 'g':
            operation_type = GET;
            break;
        default:
            sprintf(reply_msg, "UNKNOWN COMMAND\n");
            send(sockfd, reply_msg, strlen(reply_msg), 0);
            return;
    }

    // 根据操作类型执行相应操作
    switch (operation_type) {
        case TOP: {
            top_vote(redis_ctx, sockfd);
            break;
        }
        case UPDATE: {
            char *writer_id_str = strtok_r(NULL, " ", &saveptr);
            char *score_str = strtok_r(NULL, " ", &saveptr);
            if (writer_id_str && score_str) {
                int writer_id = atoi(writer_id_str);
                int score = atoi(score_str);
                if (update_vote(writer_id, score) == 0) {
                    sprintf(reply_msg, "UPDATE SUCCESS!\n");
                } else {
                    sprintf(reply_msg, "UPDATE FAILED\n");
                }
            } else {
                sprintf(reply_msg, "UPDATE FAILED: Invalid parameters\n");
            }
            break;
        }
        case DELETE_OP: {
            char *writer_id_str = strtok_r(NULL, " ", &saveptr);
            char *passwd = strtok_r(NULL, " ", &saveptr);
            if (writer_id_str && passwd) {
                int writer_id = atoi(writer_id_str);
                if (delete_vote(writer_id, passwd) == 0) {
                    sprintf(reply_msg, "DELETE SUCCESS!\n");
                } else {
                    sprintf(reply_msg, "DELETE FAILED\n");
                }
            } else {
                sprintf(reply_msg, "DELETE FAILED: Invalid parameters\n");
            }
            break;
        }
        case CREATE: {
            char *writer_name = strtok_r(NULL, " ", &saveptr);
            char *passwd = strtok_r(NULL, " ", &saveptr);
            if (writer_name && passwd) {
                create_vote(writer_name, passwd);
                sprintf(reply_msg, "CREATE SUCCESS!\n");
            } else {
                sprintf(reply_msg, "CREATE FAILED: Invalid parameters\n");
            }
            break;
        }
        case ADD: {
            char *writer_id_str = strtok_r(NULL, " ", &saveptr);
            if (writer_id_str) {
                int writer_id = atoi(writer_id_str);
                if (add_vote(writer_id) == 0) {
                    sprintf(reply_msg, "ADD SUCCESS!\n");
                } else {
                    sprintf(reply_msg, "ADD FAILED\n");
                }
            } else {
                sprintf(reply_msg, "ADD FAILED: Invalid parameters\n");
            }
            break;
        }
        case GET: {
            char *writer_id_str = strtok_r(NULL, " ", &saveptr);
            if (writer_id_str) {
                int writer_id = atoi(writer_id_str);
                int score = get_vote_score(writer_id);
                if (score >= 0) {
                    sprintf(reply_msg, "%d\n", score);
                } else {
                    sprintf(reply_msg, "GET FAILED\n");
                }
            } else {
                sprintf(reply_msg, "GET FAILED: Invalid parameters\n");
            }
            break;
        }
    }

    // 发送回复给客户端
    int sent_bytes = send(sockfd, reply_msg, strlen(reply_msg), 0);
    if (sent_bytes < 0) {
        perror("Write failed");
    }
}

void run_server() {
    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    fd_set readfds;
    int max_fd = 0;

    // 初始化数据库连接
    //init_db_connections();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        perror("ERROR on listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    max_fd = sockfd;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
//        struct timeval timeout;
//        timeout.tv_sec = 30;  // Timeout value in seconds
//        timeout.tv_usec = 0;
    while (1) {
        int nready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (nready < 0 && errno != EINTR) {
            perror("select error");
            exit(EXIT_FAILURE);
        }
        // 遍历所有套接字，检查是否有数据可读
        for (int i = 0; i <= max_fd; i++) {
            if (!FD_ISSET(i, &readfds)) {
                continue;
            }
            if (i == sockfd) {
                connfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
                if (connfd < 0) {
                    perror("ERROR on accept");
                    continue;
                }
                printf("New client connected from %s\n", inet_ntoa(cli_addr.sin_addr));
                FD_SET(connfd, &readfds); // 添加新的连接到 readfds
                if (connfd > max_fd) {
                    max_fd = connfd; // 更新最大文件描述符
                }
            } else {
                handle_client(i);
            }
        }
    }
    close(sockfd);
    close_db_connections();
}
