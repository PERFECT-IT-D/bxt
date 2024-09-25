#include "public.h"
// For close()

//struct message{
//    enum msg_type,
//    int msg_len,
//    char * msg_content,
//};
//
//char * msg2str(struct message msg){
//    return xxx;
//}
//
//struct message str2msg(char * msg){
//    return xxx;
//}

int main(int argc, char *argv[]) {
    //message 协议，thrift/protobuf
    if (argc < 2) {
        printf("Usage: %s [server|client]\n", argv[0]);
        return 1;
    }

    // 根据命令行参数决定是运行服务器还是客户端
    if (strcmp(argv[1], "server") == 0) {
        // 初始化数据库连接
        init_db_connections();
        // 运行服务器
        run_server();
        // 服务器运行结束后关闭数据库连接
        close_db_connections();
    } else if (strcmp(argv[1], "client") == 0) {
        run_client();
    } else {
        fprintf(stderr, "Invalid mode: %s\n", argv[1]);
        return 1;
    }
    return 0;
}