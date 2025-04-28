#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "functions.h"


int parse_args(int argc, char* argv[]) {

  int port = 1234;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
        port = atoi(argv[i+1]);
        i++;
    }
}
  return port;

}

// Reads from a client and writes back depending on print parameter (true or false)
void* handleConnection(void* arg) {
  struct ThreadArgs* args = (struct ThreadArgs*)arg;

  int returnval;
  char buffer[args->buffer_size];
  int client_fd = args->client_fd;

  write(client_fd, "Hello from server!\n", 19);

  while((returnval = read(client_fd, (void*)&buffer, sizeof(buffer))) > 0) {
      buffer[returnval] = '\0';

      ssize_t bytes_written = write(client_fd, (void*)buffer, (size_t)returnval);

      if (bytes_written < 0) {
        perror("write");
      }
  }
  printf("Client disconnected on fd %d\n", client_fd);
  close(client_fd);
  free(arg);

    return NULL;
} 
