
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_REQS_H
#define CHTD_REQS_H


#include "chtd.h"


enum http_method_t
{
    HTTP_METHOD_UNKNOWN,
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS
};


enum http_version_t
{
    HTTP_VERSION_UNKNOWN,
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1
};


struct reqs_t
{
    enum http_method_t  method;
    char *       method_name;
    enum http_version_t http_version;
    char *       http_version_name;
    char *reqs_line;
    char *uri;
    char *request_path;
    char *query_string;
    char *real_path;
    char *docs_root;
    char *post_data;
    int   post_size;
    int   post_read_flag;
    int   rp_header_sent;
    
    int   content_length;
    char *rp_status_line;
    struct namevalue_t *post_vars; /* Request  headers */
    struct namevalue_t *re_headers; /* Request  headers */
    struct namevalue_t *rp_headers; /* Response headers */
    struct conn_t *conn;
    struct wker_t *wker;
    struct htdx_t *htdx;
};


struct reqs_t *
reqs_new(struct conn_t *);


void
reqs_del(struct reqs_t *);


struct htdx_t *
reqs_get_htdx(struct reqs_t *);


struct conn_t *
reqs_get_conn(struct reqs_t *);


struct wker_t *
reqs_get_wker(struct reqs_t *);


int
reqs_conn_send(struct reqs_t *, void *, int);


void
reqs_throw_status(struct reqs_t *, int, char *);


int
reqs_read_post(struct reqs_t *);


int
reqs_parse(struct reqs_t *);


int
reqs_proc(struct conn_t *);


#endif
