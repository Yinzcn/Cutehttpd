
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_FCGI_PMGR_H
#define CHTD_FCGI_PMGR_H


#include "chtd.h"
#include "fcgi.h"


struct fcgi_proc_t
{
    int    port;
    int    n_conn_max;
    int    n_conn_cur;
    char  *cmdl;
    struct usa_t rsa;
    struct fcgi_proc_t *prev;
    struct fcgi_proc_t *next;
};


/* Process Manager */
struct fcgi_pmgr_t
{
    int    enablepmgr;
    int    n_conn_max;
    int    n_conn_cur;
    char   cgiextname[16];
    char   fcgid_addr[64];
    char   fcgid_port[16];
    char   fcgid_cmdl[256];
    pthread_mutex_t mutex;
    struct usa_t rsa;
    struct fcgi_proc_t *fcgi_procs;
    struct htdx_t *htdx;
    struct fcgi_pmgr_t *prev;
    struct fcgi_pmgr_t *next;
};


struct fcgi_pmgr_t *
fcgi_pmgr_add(struct htdx_t *, char *, char *, char *, char *);


void
fcgi_pmgr_del(struct fcgi_pmgr_t *);


struct fcgi_pmgr_t *
fcgi_pmgr_match(struct htdx_t *, char *);


struct fcgi_proc_t *
fcgi_pmgr_proc_spawn(struct fcgi_pmgr_t *);


struct fcgi_proc_t *
fcgi_pmgr_proc_assign(struct fcgi_pmgr_t *);


int
fcgi_pmgr_conn(struct fcgi_pmgr_t *, struct fcgi_conn_t *);


void *
chtd_set_fcgi(struct htdx_t *, char *, char *, char *, char *);


#endif
