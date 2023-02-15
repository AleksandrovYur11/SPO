#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zconf.h>

void print_errno(char* strerr) {
    if (errno) {
        fprintf(stderr, "%s. Îøèáêà: %s\n", strerr, strerror(errno));
        exit(1);
    }
}
 
void getinfo(int argc, char* argv[]) {
    struct addrinfo addrinfo;
    addrinfo.ai_family = AF_INET;
    addrinfo.ai_socktype = SOCK_STREAM;
    addrinfo.ai_protocol = IPPROTO_TCP;
    addrinfo.ai_flags = 0;
    struct addrinfo* addr;

    getaddrinfo(argv[1], argv[2], &addrinfo, &addr);
    print_errno("Using host port [paths...]");

    int sockfd = 0;
    for (struct addrinfo* a = addr; a != NULL; a = a->ai_next) {
        if ((sockfd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) == -1)
            continue;
        if (connect(sockfd, a->ai_addr, a->ai_addrlen) == 0)
            break;
        sockfd = close(sockfd);
    }
    if (sockfd <= 0) {
        fprintf(stderr, "Impossible  connect to %s:%s\n", argv[1], argv[2]);
        exit(1);
    }
    freeaddrinfo(addr);

    for (int i = 3; i < argc; ++i) {
        int a = write(sockfd, argv[i], strlen(argv[i]));
        if (a == -1) {
            return;
        }
        a = write(sockfd, "\r", 1);
        if (a == -1) {
            return;
        }
    }
    int b = write(sockfd, "\r", 1);
    if (b == -1) {
        return;
    }

    char* buf = (char*)malloc(2048 * sizeof(char));
    int len = 0;
    int bytes_read;
    char* response = NULL;
    while ((bytes_read = read(sockfd, buf, 2048)) > 0) {
        response = realloc(response, len + bytes_read);
        memcpy(response + len, buf, bytes_read);
        len += bytes_read;
        if (*(response + len - 1) == '\0') {
            break;
        }
    }
    printf("Resived from server: %s\n", response);

    close(sockfd);
}

int main(int argc, char* argv[]) {
    getinfo(argc, argv);
    return 0;
}
