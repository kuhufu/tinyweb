#include <stdlib.h>
#include <sys/mman.h>
#include "rio.h"
#include "http.h"

#define MAXLINE 512
#define MAXBUF 8192

void doit(int fd);
int parse_uri(char *uri, char *filename);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmse);

void get(int fd, Request *req);
void post(int fd, Request *req);
void echo_post(int fd, char *body);

int main(int argc, char **argv)
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;


    if (argc != 2){
        fprintf(stdout, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    listenfd = open_listenfd(port);
    printf("server listen at: %d\n", port);
    while (1){
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, &clientaddr, &clientlen);
        doit(connfd);
        close(connfd);
    }

}

void doit(int fd)
{
    char buf[MAXBUF] = {0}, *method, *uri, *version;
    Request req;

    // Read request line and headers
    recv(fd, buf, sizeof(buf), 0);
    load_request(&req, buf);

    // parse to request
    method = req.start_line.method;
    uri = req.start_line.uri;
    version = req.start_line.version;

    printf("\n%s %s %s\n", method, uri, version);

    if(strcasecmp(method, "GET") == 0)
        get(fd, &req);
    else if(strcasecmp(method, "POST") == 0)
        post(fd, &req);
    else
        clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
}

void get(int fd, Request *req)
{
    printf("get() called\n");

    struct stat sbuf;
    char filename[MAXLINE];
    char *uri = req->start_line.uri;

    // Parse URI from GET request
    parse_uri(uri, filename);
    if(stat(filename, &sbuf) < 0){
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    // Serve static content
    if(!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode)){
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
        return;
    }
    serve_static(fd, filename, sbuf.st_size);
}

void post(int fd, Request *req)
{
    printf("post() called\n");
    printf("req.body: %s\n", req->body);
    echo_post(fd, req->body);
}

void echo_post(int fd, char *content)
{
    char buf[MAXBUF] = {0};
    Response res;

    // Build the HTTP response body
    char *body = res.body;
    sprintf(body, "<html><title>Tiny Post</title>\r\n");
    sprintf(body, "%s<body>\r\n<p>%s", body, content);
    sprintf(body, "%s<hr><em>The Tiny Web server</em></p>\r\n</body>\r\n</html>\r\n", body);

    // Build the HTTP response start line and header
    write_start_line(&res, "HTTP/1.1", "200", "OK");
    write_content(&res, "text/html", strlen(body));

    // Format to HTML and write
    dump_response(&res, buf);
    send(fd, buf, strlen(buf),0);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXBUF] = {0};
    Response res;

    // Build the HTTP response body
    char *body = res.body;
    sprintf(body, "<html><title>Tiny Error</title>\r\n");
    sprintf(body, "%s<body>\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em></p>\r\n</body>\r\n</html>", body);


    // Build the HTTP response start line and header
    write_start_line(&res, "HTTP/1.1", errnum, shortmsg);
    write_content(&res, "text/html", strlen(body));

    // Format to HTML and write
    dump_response(&res, buf);
    printf(buf);
    send(fd, buf, strlen(buf), 0);
}

int parse_uri(char *uri, char *filename)
{
    strcpy(filename, ".");
    strcat(filename, uri);
    if(uri[strlen(uri)-1] == '/')
        strcat(filename, "index.html");
    return 1;
}

void serve_static(int fd, char *filename, int filesize)
{
    printf("serve_static() called\n");
    char filetype[MAXLINE], buf[MAXBUF] = {0};
    Response res;
    init_res(&res);

    int srcfd;
    char *srcp;
    get_filetype(filename, filetype);

    // Build the HTTP response start line and header
    write_start_line(&res, "HTTP/1.1", "200", "OK");
    write_content(&res, filetype, filesize);

    // Send response start line and header to client
    dump_response(&res, buf);
    send(fd, buf, strlen(buf), 0);

    // Send response body to client
    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    send(fd, srcp, filesize, 0);
    munmap(srcp, filesize);

}

void get_filetype(char *filename, char *filetype)
{
    if(strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".css"))
        strcpy(filetype, "text/css");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if(strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if(strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else
        strcpy(filetype, "text/plain");
}
