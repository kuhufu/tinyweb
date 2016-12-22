//
// Created by three on 16-12-13.
//

#ifndef TINYWEB_RIO_H
#define TINYWEB_RIO_H

#include <wchar.h>

#define BODYLEN 4096
#define LINELEN 256
// 请求
struct _req_startline{
    char method[LINELEN];
    char uri[LINELEN];
    char version[LINELEN];
};
struct _req_head{
    char host[LINELEN];
};
typedef struct request{
    struct _req_startline start_line;
    struct _req_head header;
    char body[BODYLEN];
} Request;

// 响应
struct _res_startline{
    char version[LINELEN];
    char status_code[LINELEN];
    char reason_phrase[LINELEN];
};
struct _res_head{
    char content_type[LINELEN];
    size_t content_length;
};
typedef struct response{
    struct _res_startline start_line;
    struct _res_head header;
    char body[BODYLEN];
} Response;

void init_res(Response *res);
void init_req(Request *req);
void load_request(Request *req, char *buf);
void dump_response(Response *res, char *buf);
char *readline(char *buf, char *pos);
void parse_req_header(Request *req, char *line);
void parse_start_line(Request *req, char *line);
void write_start_line(Response *res, char *version, char *status_code, char *reason_phrase);
void write_content(Response *res, char *type, size_t len);
#endif //TINYWEB_RIO_H
