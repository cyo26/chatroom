#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "list.h"

#define BACKLOG 1024

#define GET_CLINETINFO(head)                 \
    ((struct client_info *) ((void *) head - \
                             (long) &((struct client_info *) 0)->list))

struct client_info {
    int fd;
    socklen_t addr_len;
    struct sockaddr addr;
    struct list_head list;
};

struct list_head head;

static int open_listenfd(int port)
{
    int listenfd, optval = 1;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminate "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
                   sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to given port. */
    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(LOCALHOST),
        .sin_port = htons((unsigned short) port),
        .sin_zero = {0},
    };

    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    if (listen(listenfd, BACKLOG) < 0)
        return -1;

    return listenfd;
}

/* broadcast the mesg to all the clients */
static void broadcast(char *mesg, int len, struct list_head *sender)
{
    printf("%s", mesg);
    struct list_head *iter;
    list_for_each (&head, iter) {
        if (iter == sender)
            continue;
        int fd = GET_CLINETINFO(iter)->fd;
        send(fd, mesg, len, 0);
    }
}

static void *client_handler(void *arg)
{
    struct client_info *client = (struct client_info *) arg;
    char buf[128], mesg[128], name[16];

    memset(name, 0, 16 * sizeof(char));

    send(client->fd, "Your name: ", 16, 0);
    if (recv(client->fd, name, 16, 0) <= 0)
        goto out;

    /* Replace '\n' with '\0' */
    char *ptr;
    for (ptr = name; *ptr != '\n'; ptr++)
        ;
    *ptr = '\0';

    snprintf(mesg, 128, "%s join the chat room\n", name);
    broadcast(mesg, 128, &client->list);

    while (1) {
        memset(buf, 0, 128 * sizeof(char));
        if (recv(client->fd, buf, 128, 0) <= 0)
            break;
        snprintf(mesg, 128, "%s: %s", name, buf);
        broadcast(mesg, 128, &client->list);
    }

    snprintf(mesg, 128, "%s leave the chat room\n", name);
    broadcast(mesg, 128, &client->list);

out:
    close(client->fd);
    list_del(&client->list);
    free(client);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return -1;

    int port = atoi(argv[1]);
    int listenfd = open_listenfd(port);

    list_init(&head);

    while (1) {
        struct client_info *client = malloc(sizeof(struct client_info));
        pthread_t thread;
        client->fd = accept(listenfd, &client->addr, &client->addr_len);
        list_add(&head, &client->list);
        pthread_create(&thread, NULL, &client_handler, (void *) client);
    }
    return 0;
}