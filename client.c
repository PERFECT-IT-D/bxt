#include "public.h"


#define PORT 8081

void run_client() {
    int cli_sockfd;
    struct sockaddr_in serv_addr;
    char message[256];
    char server_reply[256] = {0};

    // 客户端代码
    cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (cli_sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(cli_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed \n");
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞
    //fcntl(cli_sockfd, F_SETFL, O_NONBLOCK);

    fd_set readfds;
    int max_fd = 0;
    FD_ZERO(&readfds);
    FD_SET(cli_sockfd, &readfds);
    max_fd = cli_sockfd;

    while (1) {
        printf("Enter operation (TOP,CREATE <writer_name> <passwd> ,ADD <writer_id> , DELETE <writer_id><passwd>, UPDATE <writer_id> <score>, GET <writer_id>): \n");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character

        // Send message to server
        if (send(cli_sockfd, message, strlen(message), 0) < 0) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }

        // Set a timeout for select
//        struct timeval timeout;
//        timeout.tv_sec = 30;  // Timeout value in seconds
//        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(cli_sockfd, &readfds);

        // Use select to wait for server response or timeout
        int nready = select(cli_sockfd + 1, &readfds, NULL, NULL, NULL);
        if (nready < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, retry select
                continue;
            } else {
                perror("select error");
                exit(EXIT_FAILURE);
            }
        } else if (nready == 0) {
            printf("select timed out!\n");
        } else {
            // Check if there is data to read
            if (FD_ISSET(cli_sockfd, &readfds)) {
                int str_len = recv(cli_sockfd, server_reply, sizeof(server_reply) - 1, 0);
                if (str_len < 0) {
                    perror("recv failed");
                    exit(EXIT_FAILURE);
                } else if (str_len == 0) {
                    printf("Server closed the connection.\n");
                    break;
                }
                server_reply[str_len] = '\0';
                printf("Server reply: %s\n", server_reply);
            }
        }
    }

// Close the socket
    close(cli_sockfd);
}