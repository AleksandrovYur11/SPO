#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <zconf.h>
#include <netinet/in.h>
#include <dirent.h>

void print_errno(char* strerr) {
    if (errno) {
        fprintf(stderr, "%s. Error: %s\n", strerr, strerror(errno));
        exit(1);
    }
}

void print_dir(int fd, const char* dirname) {
    errno = 0;
    DIR* dir = opendir(dirname);

    char* buf = (char*)malloc(2048);
    sprintf(buf, "Folder Contents \"%s\":\n", dirname);
    int a = write(fd, buf, strlen(buf));
    if (a == -1) {
        return;
    }
    free(buf);

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        char* buf = (char*)malloc(2048);
        sprintf(buf, "%s\n", ent->d_name);
        int b = write(fd, buf, strlen(buf));
        if (b == -1) {
            return;
        }
        free(buf);
    }
    char nn[] = "\n\n";
    int c = write(fd, nn, strlen(nn));
    if (c == -1) {
        return;
    }
    closedir(dir);
}

unsigned int check_args(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Using: ./server port");
        exit(1);
    }
    char* p;
    unsigned int port = (int)strtol(argv[optind], &p, 10);
    if (errno != 0 || *p != '\0' || port > 65635) {
        fprintf(stderr, "Invalid port specified.\n");
        exit(1);
    }
    return port;
}

void read_client(int client) {
    int len = 0;
    char* buf = (char*)malloc(2048 * sizeof(char));
    int bytes_read;
    char* request = NULL;
    while ((bytes_read = read(client, buf, 2048)) > 0) {
        request = realloc(request, len + bytes_read);
        memcpy(request + len, buf, bytes_read);
        len += bytes_read;
        printf(" request: \"%s\"\n", request);
        if (*(request + len - 1) == '\0' || *(request + len - 1) == (char)0x0a || *(request + len - 1) == (char)0x0d) {
            break;
        }
    }

    char* dirs = strtok(request, "\r");
    while (dirs != NULL) {
        print_dir(client, dirs);
        dirs = strtok(NULL, "\r");
    }
    int d = write(client, "\0", 1);
    if (d == -1) {
        return;
    }
    free(request);
    close(client);
}

void start_server(unsigned int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    print_errno("Unable to create socket");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    struct in_addr sin_addr;
    sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr = sin_addr;
    bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    print_errno("It is not possible to bind a local address to a socket");
    listen(sockfd, 10);
    print_errno("Unable to start listening for connections");

    printf("Listening on %d\n", port);

    for (;;) {
        int client = accept(sockfd, NULL, NULL);
        if (!fork()) {
            print_errno("Unable to create a subprocess");
            printf("Client is connected: %d\n", client);
            read_client(client);
        }

    }
}

int main(int argc, char* argv[]) {
    start_server(check_args(argc, argv));
    return 0;
}
