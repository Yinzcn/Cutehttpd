
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_HTTP_H
#define CHTD_HTTP_H


int
set_status_line(struct reqs_t *, char *);


int
set_http_status(struct reqs_t *, int);


int
set_http_header(struct reqs_t *, char *, char *);


int
set_http_header_x(struct reqs_t *, char *, char *, ...);


int
set_keep_alive(struct reqs_t *, int);


char *
get_http_header(struct reqs_t *, char *);


int
send_http_header(struct reqs_t *);


int
send_http_chunk(struct reqs_t *, void *, int);


int
send_http_chunk_end(struct reqs_t *);


#endif
