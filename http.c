//
// Created by three on 16-12-13.
//

#include <string.h>
#include <stdio.h>
#include "http.h"

void init_req(Request *req)
{
    bzero(req->body, sizeof(req->body));
    bzero(req->start_line.method, sizeof(req->start_line.method));
    bzero(req->start_line.uri, sizeof(req->start_line.uri));
    bzero(req->start_line.version, sizeof(req->start_line.version));
    bzero(req->header.host, sizeof(req->header.host));

}

void init_res(Response *res){
    bzero(res->body, sizeof(res->body));
    bzero(res->start_line.version, sizeof(res->start_line.version));
    bzero(res->start_line.reason_phrase, sizeof(res->start_line.reason_phrase));
    bzero(res->start_line.status_code, sizeof(res->start_line.status_code));
    bzero(res->header.content_type, sizeof(res->header.content_type));
}
void load_request(Request *req, char *buf)
{
    char line[LINELEN] = {0};
    char *pos = buf;

    pos = readline(line, pos);
    parse_start_line(req, line);

    while (*(pos + 1) != '\n' || *(pos - 1) != '\n' && *pos != '\0'){
        bzero(line, sizeof(line));
        pos = readline(line, pos);
        parse_req_header(req, line);
    }
    bzero(req->body, sizeof(req->body));
    char *bodyptr = strstr(buf, "\r\n\r\n");
    bodyptr += 4;
    strcpy(req->body, bodyptr);
}

void dump_response(Response *res, char *buf)
{
    // start line
    strcat(buf, res->start_line.version);
    strcat(buf, " ");
    strcat(buf, res->start_line.status_code);
    strcat(buf, " ");
    strcat(buf, res->start_line.reason_phrase);
    strcat(buf, "\r\n");

    // header
    char len[20];
    sprintf(len, "%lu", res->header.content_length);
    strcat(buf, "Content-type: ");
    strcat(buf, res->header.content_type);
    strcat(buf, "\r\n");
    strcat(buf, "Content-length: ");
    strcat(buf, len);

    // delmiter of header and body
    strcat(buf, "\r\n\r\n");

    // body
    if(strlen(res->body))
        strcat(buf, res->body);
}

void parse_start_line(Request *req, char *line)
{
    char method[LINELEN] = {0};
    char uri[LINELEN] = {0};
    char version[LINELEN] = {0};
    sscanf(line, "%s %s %s", method, uri, version);

    strcpy(req->start_line.method, method);
    strcpy(req->start_line.uri, uri);
    strcpy(req->start_line.version, version);
}

void parse_req_header(Request *req, char *line)
{
    char key[LINELEN] = {0};
    char value[LINELEN] = {0};

    char *pos = strstr(line, ": ");
    strcpy(value, pos+2);
    strncpy(key, line, pos - line);
    if(strcasecmp("Host", key) == 0){
        strcpy(req->header.host, value);
    }
}

char *readline(char *buf, char *pos){
    char *p;
    for(p = pos; *p != '\n'; p++){
        *buf++ = *p;
    }
    return p + 1;
}

void write_start_line(Response *res, char *version, char *status_code, char *reason_phrase)
{
    strcpy(res->start_line.version, version);
    strcpy(res->start_line.status_code, status_code);
    strcpy(res->start_line.reason_phrase, reason_phrase);
}

void write_content(Response *res, char *type, size_t len)
{
    strcpy(res->header.content_type, type);
    res->header.content_length = len;
}