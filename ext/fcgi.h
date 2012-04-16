
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_FCGI_H
#define CHTD_FCGI_H


#include "chtd.h"
#include "fastcgi.h"


struct fcgi_reqs_t
{
    int         requestId;
    FCGI_Header re_header;
    FCGI_Header rp_header;
    int         rp_requestId;
    int         rp_contentLength;
    int         tran_http_header_flag;
    int    params_buffsize;
    int    params_datasize;
    char  *params_databody;
    struct bufx_t       *stderrbufx;
    struct bufx_t       *stdoutbufx;
    struct fcgi_conn_t  *fcgi_conn;
    struct conn_t       *http_conn;
    struct reqs_t       *http_reqs;
    struct fcgi_pmgr_t  *fcgi_pmgr;
    struct htdx_t       *htdx;
};


struct fcgi_conn_t
{
    struct sock_t        sock;
    struct fcgi_reqs_t  *fcgi_reqs;
    struct reqs_t       *http_reqs;
    struct fcgi_pmgr_t  *fcgi_pmgr;
    struct htdx_t       *htdx;
};


struct fcgi_conn_t *
fcgi_conn_new(struct reqs_t *);


void
fcgi_conn_del(struct fcgi_conn_t *);


void
fcgi_conn_close(struct fcgi_conn_t *);


int
fcgi_conn_send(struct fcgi_conn_t *, void *, int);


int
fcgi_conn_recv(struct fcgi_conn_t *, void *, int);


struct fcgi_reqs_t *
fcgi_reqs_new(struct fcgi_conn_t *);


void
fcgi_reqs_del(struct fcgi_reqs_t *);


int
fcgi_send_header(struct fcgi_reqs_t *);


int
fcgi_recv_header(struct fcgi_reqs_t *);


int
fcgi_send_padding(struct fcgi_reqs_t *, int);


int
fcgi_recv_padding(struct fcgi_reqs_t *, int);


int
fcgi_send_begin_record(struct fcgi_reqs_t *);


int
fcgi_add_params(struct fcgi_reqs_t *, char *, char *);


int
fcgi_send_params(struct fcgi_reqs_t *);


int
fcgi_send_stdin(struct fcgi_reqs_t *, char *, int);


int
fcgi_recv_http_header(struct fcgi_conn_t *, char *, int);


int
fcgi_trans_header(struct fcgi_reqs_t *, char *);


int
fcgi_trans_stdout(struct fcgi_reqs_t *);


int
fcgi_trans_stderr(struct fcgi_reqs_t *);


int
fcgi_reqs_done(struct fcgi_reqs_t *);


int
fcgi_reqs_proc(struct fcgi_pmgr_t *, struct reqs_t *);


#endif
