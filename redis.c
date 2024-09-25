#include "public.h"

extern redisContext *redis_ctx;

void init_redis() {
    redis_ctx = redisConnect("127.0.0.1", 6379);
    if (redis_ctx == NULL || redis_ctx->err) {
        fprintf(stderr, "Redis connect error: %s\n", redis_ctx ? redis_ctx->errstr : "unknown error");
        exit(EXIT_FAILURE);
    }
}

void top_redis(redisContext *redis_ctx, int client_socket) {
    char cmd[128];
    sprintf(cmd, "ZREVRANGE writers_scores 0 2 WITHSCORES");

    redisReply *reply = redisCommand(redis_ctx, cmd);
    if (reply == NULL) {
        fprintf(stderr, "Redis command failed: %s\n", redis_ctx->errstr);
        return;
    } else if (reply->type == REDIS_REPLY_ARRAY) {
        // 构建要发送给客户端的消息
        char message[1024];
        snprintf(message, sizeof(message), "Top 3 votes:\n");
        size_t message_len = strlen(message);

        for (size_t i = 0; i < reply->elements; i += 2) {
            char entry[256];
            snprintf(entry, sizeof(entry), "Writer: %s, Score: %lld\n",
                     reply->element[i]->str, reply->element[i + 1]->integer);
            // 将每个条目追加到消息中
            if (message_len + strlen(entry) < sizeof(message)) {
                strcat(message, entry);
                message_len += strlen(entry);
            } else {
                fprintf(stderr, "Message buffer overflow\n");
                break;
            }
        }

        // 发送构建好的消息给客户端
        if (send_data_to_client(client_socket, message, message_len) != 0) {
            fprintf(stderr, "Failed to send data to client\n");
        }
    } else {
        fprintf(stderr, "Unexpected reply type: %d\n", reply->type);
    }

    // 清理回复对象
    freeReplyObject(reply);
}

void create_redis(int writer_id) {
    char cmd[128];
    sprintf(cmd, "ZADD writer_score 10 %d", writer_id);

    // 执行 Redis 命令
    redisReply *reply = redisCommand(redis_ctx, cmd);

    // 检查 Redis 命令是否执行成功
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis add vote error: %s\n", reply ? reply->str : redis_ctx->errstr);
        freeReplyObject(reply);
    } else {
        printf("Vote added successfully for writer_id: %d\n", writer_id);
    }

    // 清理
    freeReplyObject(reply);
}

void add_redis(int writer_id) {
    char cmd[128];
    int new_score = 10;
    sprintf(cmd, "ZINCRBY writer_score %d %d", new_score, writer_id);

    // 执行 Redis 命令
    redisReply *reply = redisCommand(redis_ctx, cmd);

    // 检查 Redis 命令是否执行成功
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis add vote error: %s\n", reply ? reply->str : redis_ctx->errstr);
        freeReplyObject(reply);
        return;
    }

    // 释放 Redis 命令的回复对象
    freeReplyObject(reply);
}

int update_redis(int writer_id, int new_score) {
    char cmd[128];
    sprintf(cmd, "ZADD writer_score %d %d", new_score, writer_id);

    redisReply *reply = redisCommand(redis_ctx, cmd);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis update vote error: %s\n", reply ? reply->str : redis_ctx->errstr);
        freeReplyObject(reply);
        return -1;
    }
    freeReplyObject(reply);
    return 0;
}

int get_vote_score_redis(int writer_id) {
    char cmd[64]; // 确保这个数组足够大以存储命令
    sprintf(cmd, "ZSCORE writer_score %d", writer_id);
    // 执行 Redis 命令
    redisReply *reply = redisCommand(redis_ctx, cmd);

    // 检查 Redis 命令是否执行成功
    if (!reply) {
        fprintf(stderr, "Redis command failed: %s\n", redis_ctx->errstr);
        return -1;
    } else if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis get vote score error: %s\n", reply->str);
        freeReplyObject(reply);
        return -1;
    } else if (reply->type == REDIS_REPLY_STRING) {
        // 尝试将回复转换为整数
        int score = atoi(reply->str);
        //printf("分数为: %d\n",score);
        freeReplyObject(reply);
        return score;
    } else if (reply->type == REDIS_REPLY_NIL) {
        // 如果哈希表中没有找到 writer_id，返回 0 或适当的错误代码
        freeReplyObject(reply);
        return 0; // 或者返回 -1 表示未找到
    } else {
        // 其他情况，可能是未知的回复类型
        fprintf(stderr, "Unexpected Redis reply type: %d\n", reply->type);
        freeReplyObject(reply);
        return -1;
    }
}

int delete_redis(int writer_id) {
    char cmd[128];
    sprintf(cmd, "ZREM writer_score %d", writer_id);
    // 执行 Redis 命令
    redisReply *reply = redisCommand(redis_ctx, cmd);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis delete vote error: %s\n", reply ? reply->str : redis_ctx->errstr);
        freeReplyObject(reply);
        return -1;
    }
}