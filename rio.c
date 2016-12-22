//
// Created by three on 16-12-10.
//

#include "rio.h"

int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if((hp = gethostbyname(hostname)) == NULL)
        return -2;

    // fill in the server's IP address and port
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
          (char *)&serveraddr.sin_addr.s_addr,
          hp->h_length
    );
    serveraddr.sin_port = htons(port);

    if(connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

int open_listenfd(int port)
{
    int listenfd;
    int optval = 1;
    struct sockaddr_in serveraddr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("socket\n");
        return -1;
    }

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))){
        printf("setsocketopt\n");
        return -1;
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);
    if(bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0){
        printf("bind\n");
        return -1;
    }

    if(listen(listenfd, 1024) < 0){
        printf("listen\n");
        return -1;
    }

    return listenfd;
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    size_t nread;
    char *bufp = usrbuf;

    while (nleft > 0){
        nread = read(fd, bufp, nleft);
        printf("nread = %d\n", nread);
        if(nread == 0 || nread < 0){
            return -1;
        }
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    ssize_t nleft = n;
    ssize_t nwrite;
    char *bufp = usrbuf;

    while (nleft > 0){
        nwrite = write(fd, bufp, nleft);
        if(nwrite == 0 || nwrite < 0){
            return -1;
        }
        nleft -= nwrite;
        bufp += nwrite;
    }
    return n;
}

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n)
{
    int cnt;
    while(rp->rio_cnt <= 0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            return -1;
        }else if(rp->rio_cnt == 0){
            return 0;
        }else{
            rp->rio_bufptr = rp->rio_buf;
        }
    }

    cnt = n;
    if(rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    for(n = 1; n < maxlen; n++){
        rc = rio_read(rp, &c, 1);
        if(rc == 1){
            *bufp++ = c;
            if(c == '\n')
                break;
        }else if(rc == 0){
            if(n == 1)
                return 0;
            else
                break;
        }else{
            return -1;
        }
    }
    *bufp = 0;
    return n;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0){
        nread = rio_read(rp, bufp, nleft);
        if(nread < 0){
            return -1;
        }else if(nread = 0){
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;
}