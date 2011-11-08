
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
    int    n_conn_curr;
    struct usa_t rsa;
    struct fcgi_proc_t *prev;
    struct fcgi_proc_t *next;
}


struct fcgi_pmgr_t   /* Process Manager */
{
    int    n_conn_max;
    int    n_conn_curr;
    char   extname[16];
    char   fcgid_addr[64];
    char   fcgid_port[16];
    char   fcgid_cmdl[256];
    struct usa_t rsa;
    struct htdx_t *htdx;
    struct fcgi_proc_t *fcgi_pmgrs;
    struct fcgi_pmgr_t *prev;
    struct fcgi_pmgr_t *next;
};


