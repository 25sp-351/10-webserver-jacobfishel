// Create a server that listens on a port (default: 80, changeable with a "-p" option).

// Accept HTTP/1.1. One message at a time, no pipelining. 

// Always:

// set the content-type appropriately when possible
// set the content-length correctly
// Check the method (should be GET for both of these)
// Implement "/static" which allows the requester to specify the name of a file in a "/static" directory. Ex: "/static/images/rex.png", returns the binary file that is there accessible to your server.

// Implement "/calc" which returns text or HTML, operating on the path components in the request

// /calc/add/<num1>/<num2>
// /calc/mul/<num1>/<num2>
// /calc/div/<num1>/<num2>
// Make it multithreaded!

// Test by hitting it from your browser. And from telnet. And from Postman or ThunderClient.

// DESIGN GOALS (as in "I'm expecting this")

// Modules for requests and responses
// clean interfaces!
// Extra credit: (25) Implement pipelining.

// can receive multiple client requests before server response is generated
// (implement a "/sleep/<seconds>" path to test this)


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
#define BUFFER_SIZE 1024



int main(int argc, char* argv[]) {

    struct Config config = parse_args(argc, argv);
    int port = config.port;
    int print = config.verbose;

    // Initialize the socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in socket_address;

    memset(&socket_address, '\0', sizeof(socket_address));

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    printf("Binding to port %d\n", port);

    int returnval;
    returnval = bind(   //associates the socket with the specified IP/port
        socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address));
    
    if (returnval < 0) {
        perror("bind");
        close(socket_fd);
        return 1;
    }
    printf("Successfully binded to port %d\n", port);


        //listn in the socket for some love up to 10 
    returnval = listen(socket_fd, LISTEN_BACKLOG);

    if (returnval < 0) {
        perror("listen");
        close(socket_fd);
        return 1;
    }

    printf("Server listening on port: %d\n", port);
    

    while (1) {
        pthread_t thread;
        int* client_fd_buf = malloc(sizeof(int));

        // Respond when someone connects
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        *client_fd_buf = accept(
                socket_fd, (struct sockaddr*)&client_address, &client_address_len);  //accept by linking the incoming request to the fd and client variables

        if (*client_fd_buf < 0) {
            perror("accept");
            continue;
        }

        struct ThreadArgs* args = malloc(sizeof(struct ThreadArgs));
        args->client_fd = *client_fd_buf;
        args->buffer_size = BUFFER_SIZE;
        args->print = print;

        printf("Accepted a connection on %d\n", *client_fd_buf);
        pthread_create(&thread, NULL, (void* (*)(void*))handleConnection,
                (void*)args);
        pthread_detach(thread);

    }   
    close(socket_fd);

    return 0;
}