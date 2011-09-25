
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_FCGI_H
#define CHTD_FCGI_H


#include "cutehttpd.h"
#include "fastcgi.h"


struct fcgi_proc_t
{
    struct usa_t rsa;
    int    n_conn_max;
    int    n_conn_curr;
    struct fcgi_proc_t *prev;
    struct fcgi_proc_t *next;
}


struct fcgi_pmgr_t   /* Process Manager */
{
    int    n_conn_max;
    int    n_conn_curr;
    char   cgiextname  [16];
    char   fcgid_addr  [64];
    char   fcgid_port  [16];
    char   fcgid_cmdl [256];
    struct fcgi_proc_t *procs;
    struct usa_t rsa;
    struct htdx_t *htdx;
};


