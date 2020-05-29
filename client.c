#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

static int open_clientfd(int port)
{
    int fd;

    /* Create a socket descriptor */
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to given port. */
    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(LOCALHOST),
        .sin_port = htons((unsigned short) port),
        .sin_zero = {0},
    };

    /* Connect ro local host server*/
    if (connect(fd, (struct sockaddr *) &serveraddr,
                sizeof(struct sockaddr_in)) < 0)
        return -1;

    return fd;
}

static int fd;

static void *print_mesg(void *arg)
{
    char mesg[128];
    while (1) {
        memset(mesg, 0, 128 * sizeof(char));
        recv(fd, mesg, 128, 0);
        printf("%s", mesg);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return -1;

    int port = atoi(argv[1]);
    fd = open_clientfd(port);

    pthread_t thread;
    pthread_create(&thread, NULL, &print_mesg, NULL);

    while (1) {
        char mesg[128];
        memset(mesg, 0, 128 * sizeof(char));
        fgets(mesg, 128, stdin);
        send(fd, mesg, 128, 0);
    }
    return 0;
}