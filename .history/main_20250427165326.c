// main.c
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "functions.h"

#define LISTEN_BACKLOG 10
#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    int port = parse_args(argc, argv);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    printf("Binding to port %d\n", port);
    if (bind(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0) {
        perror("bind");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, LISTEN_BACKLOG) < 0) {
        perror("listen");
        close(socket_fd);
        return 1;
    }

    printf("Server listening on port: %d\n", port);

    while (1) {
        pthread_t thread;
        int* client_fd_buf = malloc(sizeof(int));

        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        *client_fd_buf = accept(socket_fd, (struct sockaddr*)&client_address, &client_address_len);
        if (*client_fd_buf < 0) {
            perror("accept");
            free(client_fd_buf);
            continue;
        }

        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->client_fd = *client_fd_buf;
        args->buffer_size = BUFFER_SIZE;

        printf("Accepted a connection on %d\n", *client_fd_buf);
        pthread_create(&thread, NULL, (void* (*)(void*))handleConnection, (void*)args);
        pthread_detach(thread);

        free(client_fd_buf);
    }

    close(socket_fd);
    return 0;
}
