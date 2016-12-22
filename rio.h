//
// Created by three on 16-12-10.
//
#ifndef WEBSERVER_CSAPP_H
#define WEBSERVER_CSAPP_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#define RIO_BUFSIZE 8192

ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

typedef struct {
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
} rio_t;

void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

typedef struct sockaddr SA;
int open_clientfd(char *hostname, int port);
int open_listenfd(int port);
#endif //WEBSERVER_CSAPP_H
