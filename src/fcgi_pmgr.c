
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "fcgi.h"


struct fcgi_pmgr_t *
fcgi_pmgr_new(struct htdx_t *htdx, char *extname, char *sz_addr, char *sz_port, char *sz_cmdl)
{
    struct fcgi_pmgr_t *fcgi_pmgr = calloc(1, sizeof(struct fcgi_pmgr_t));

    fcgi_pmgr->n_conn_max = 64;

    strncpy(fcgi_pmgr->cgiextname, extname,  15);
    strncpy(fcgi_pmgr->fcgid_addr, sz_addr,  63);
    strncpy(fcgi_pmgr->fcgid_port, sz_port,  15);
    strncpy(fcgi_pmgr->fcgid_cmdl, sz_cmdl, 255);

    fcgi_pmgr->rsa.u.sin.sin_family      = AF_INET;
    fcgi_pmgr->rsa.u.sin.sin_addr.s_addr =  inet_addr(fcgi_pmgr->fcgid_addr);
    fcgi_pmgr->rsa.u.sin.sin_port        = htons(atoi(fcgi_pmgr->fcgid_port));

    fcgi_pmgr->htdx = htdx;
    return fcgi_pmgr;
}


void
fcgi_pmgr_del(struct fcgi_pmgr_t *fcgi_pmgr)
{
    if (!fcgi_pmgr)
    {
        return;
    }
    fcgi_pmgr->htdx->fcgi_pmgr = NULL;
    free(fcgi_pmgr);
}


int
chtd_set_fcgi(struct htdx_t *htdx, char *extname, char *sz_addr, char *sz_port, char *sz_cmdl)
{
    int port = atoi(sz_port);

    if (port > 65535 || port < 1)
    {
        sz_port = "9000";
    }

    if (strlen(sz_addr) == 0)
    {
        sz_addr = "0.0.0.0";
    }

    char *temp = str_replace("@addr@", sz_addr, sz_cmdl);
    char *cmdl = str_replace("@port@", sz_port, temp);
    fcgi_pmgr_del(htdx->fcgi_pmgr);
    htdx->fcgi_pmgr = fcgi_pmgr_new(htdx, extname, sz_addr, sz_port, cmdl);
    free(temp);
    free(cmdl);
    return 1;
}


