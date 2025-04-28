// functions.c
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>

#include "functions.h"

// Parse command-line arguments
int parse_args(int argc, char* argv[]) {
    int port = 80; // default port
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++;
        }
    }
    return port;
}

// Handle a client connection
void* handleConnection(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    int client_fd = args->client_fd;
    char buffer[args->buffer_size];

    int returnval = read(client_fd, buffer, sizeof(buffer) - 1);
    if (returnval <= 0) {
        close(client_fd);
        free(arg);
        return NULL;
    }
    buffer[returnval] = '\0'; // null-terminate

    printf("Request:\n%s\n", buffer);

    // Parse first line
    char method[8], path[1024], version[16];
    sscanf(buffer, "%s %s %s", method, path, version);

    if (strcmp(method, "GET") != 0) {
        const char* bad_method = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        write(client_fd, bad_method, strlen(bad_method));
        close(client_fd);
        free(arg);
        return NULL;
    }

    if (strncmp(path, "/static/", 8) == 0) {
        handle_static(client_fd, path + 8);
    } else if (strncmp(path, "/calc/", 6) == 0) {
        handle_calc(client_fd, path + 6);
    } else {
        const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(client_fd, not_found, strlen(not_found));
    }

    close(client_fd);
    free(arg);
    return NULL;
}

// Serve a static file
void handle_static(int client_fd, const char* filepath) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "static/%s", filepath);

    FILE* fp = fopen(fullpath, "rb");
    if (!fp) {
        const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(client_fd, not_found, strlen(not_found));
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    char* file_buffer = malloc(filesize);
    fread(file_buffer, 1, filesize, fp);
    fclose(fp);

    // Guess MIME type
    const char* mime_type = "application/octet-stream";
    if (strstr(filepath, ".html")) mime_type = "text/html";
    else if (strstr(filepath, ".png")) mime_type = "image/png";
    else if (strstr(filepath, ".jpg") || strstr(filepath, ".jpeg")) mime_type = "image/jpeg";
    else if (strstr(filepath, ".txt")) mime_type = "text/plain";

    // Send header
    char header[1024];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "\r\n", mime_type, filesize);

    write(client_fd, header, strlen(header));
    write(client_fd, file_buffer, filesize);

    free(file_buffer);
}

// Handle calculation path
void handle_calc(int client_fd, const char* path) {
    char operation[10];
    int num1, num2;

    sscanf(path, "%[^/]/%d/%d", operation, &num1, &num2);

    int result;
    if (strcmp(operation, "add") == 0) {
        result = num1 + num2;
    } else if (strcmp(operation, "mul") == 0) {
        result = num1 * num2;
    } else if (strcmp(operation, "div") == 0) {
        if (num2 == 0) {
            const char* bad_request = "HTTP/1.1 400 Bad Request\r\n\r\nDivision by zero";
            write(client_fd, bad_request, strlen(bad_request));
            return;
        }
        result = num1 / num2;
    } else {
        const char* bad_request = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid operation";
        write(client_fd, bad_request, strlen(bad_request));
        return;
    }

    char body[128];
    snprintf(body, sizeof(body), "<html><body>Result: %d</body></html>", result);

    char header[1024];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "\r\n", strlen(body));

    write(client_fd, header, strlen(header));
    write(client_fd, body, strlen(body));
}
