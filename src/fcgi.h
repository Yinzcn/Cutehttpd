
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_FCGI_H
#define CHTD_FCGI_H


#include "cutehttpd.h"
#include "fastcgi.h"


struct fcgi_pmgr_t   /* Process Manager */
{
    int    n_conn_max;
    int    n_conn_curr;
    char   cgiextname  [16];
    char   fcgid_addr  [64];
    char   fcgid_port  [16];
    char   fcgid_cmdl [256];
    struct usa_t rsa;
    struct htdx_t *htdx;
};


struct fcgi_reqs_t
{
    int         requestId;
    FCGI_Header re_header;
    FCGI_Header rp_header;
    int         rp_requestId;
    int         rp_contentLength;
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
    struct sock_t       sock;
    struct bufx_t       *recvbufx;
    struct fcgi_reqs_t  *fcgi_reqs;
    struct reqs_t       *http_reqs;
    struct fcgi_pmgr_t  *fcgi_pmgr;
    struct htdx_t       *htdx;
};


struct fcgi_pmgr_t *
fcgi_pmgr_new(struct htdx_t *, char *, char *, char *, char *);


void
fcgi_pmgr_del(struct fcgi_pmgr_t *);


int
chtd_set_fcgi(struct htdx_t *, char *, char *, char *, char *);


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
fcgi_reqs_new(struct reqs_t *);


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
fcgi_reqs_proc(struct reqs_t *, struct vhost_t *);


#endif
