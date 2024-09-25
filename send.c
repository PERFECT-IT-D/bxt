#include "public.h"

int send_data_to_client(int client_socket, const char *data, size_t length) {
    if (send(client_socket, data, length, 0) == -1) {
        perror("Failed to send data to client");
        return -1;
    }
    return 0;
}