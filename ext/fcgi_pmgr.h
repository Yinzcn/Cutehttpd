
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_FCGI_H
#define CHTD_FCGI_H


#include "chtd.h"
#include "fastcgi.h"


struct fcgi_proc_t
{
    int    port;
    int    n_conn_max;
    int    n_conn_cur;
    struct usa_t rsa;
    struct fcgi_proc_t *prev;
    struct fcgi_proc_t *next;
}


/* Process Manager */
struct fcgi_pmgr_t
{
    int    n_conn_max;
    int    n_conn_cur;
    char   extname[16];
    char   fcgi_addr[64];
    char   fcgi_port[16];
    char   fcgi_cmdl[256];
    pthread_mutex_t mutex;
    struct usa_t rsa;
    struct fcgi_proc_t *fcgi_pmgrs;
    struct htdx_t *htdx;
    struct fcgi_pmgr_t *prev;
    struct fcgi_pmgr_t *next;
};
