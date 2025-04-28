#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct Config {
  int port;
  int verbose;
};

struct ThreadArgs {
  int client_fd;
  int buffer_size;
  int print;
};

struct Config parse_args(int argc, char* argv[]);
void* handleConnection(void* arg);


#endif