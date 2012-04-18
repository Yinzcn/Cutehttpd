
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "fcgi.h"
#include "fcgi_pmgr.h"
#include "w32_envblk.c"


struct fcgi_pmgr_t *
fcgi_pmgr_add(struct htdx_t *htdx, char *extname, char *sz_addr, char *sz_port, char *sz_cmdl) {
    struct fcgi_pmgr_t *fcgi_pmgr = calloc(1, sizeof(struct fcgi_pmgr_t));
    if (!fcgi_pmgr) {
        chtd_cry(htdx, "fcgi_pmgr_new() failed!");
        return NULL;
    }
    strncpy(fcgi_pmgr->cgiextname, extname,  15);
    strncpy(fcgi_pmgr->fcgid_addr, sz_addr,  63);
    strncpy(fcgi_pmgr->fcgid_port, sz_port,  15);
    strncpy(fcgi_pmgr->fcgid_cmdl, sz_cmdl, 255);
    if (sz_cmdl && strlen(sz_cmdl)) {
        fcgi_pmgr->enablepmgr = 1;
    }
    fcgi_pmgr->rsa.u.sin.sin_family = AF_INET;
    fcgi_pmgr->rsa.u.sin.sin_addr.s_addr = inet_addr(sz_addr);
    fcgi_pmgr->rsa.u.sin.sin_port = htons(atoi(sz_port));
    if (htdx->fcgi_pmgrs) {
        fcgi_pmgr->prev = htdx->fcgi_pmgrs->prev;
        fcgi_pmgr->next = htdx->fcgi_pmgrs;
        fcgi_pmgr->prev->next = fcgi_pmgr;
        fcgi_pmgr->next->prev = fcgi_pmgr;
    } else {
        fcgi_pmgr->prev  = fcgi_pmgr;
        fcgi_pmgr->next  = fcgi_pmgr;
        htdx->fcgi_pmgrs = fcgi_pmgr;
    }
    return fcgi_pmgr;
}


void
fcgi_pmgr_del(struct fcgi_pmgr_t *fcgi_pmgr)
{
    if (fcgi_pmgr == NULL) {
        return;
    } else
    if (fcgi_pmgr == fcgi_pmgr->next) {
        fcgi_pmgr->htdx->fcgi_pmgrs = NULL;
        free(fcgi_pmgr);
        return;
    } else {
        if (fcgi_pmgr == fcgi_pmgr->htdx->fcgi_pmgrs) {
            fcgi_pmgr->htdx->fcgi_pmgrs = fcgi_pmgr->next;
        }
        fcgi_pmgr->prev->next = fcgi_pmgr->next;
        fcgi_pmgr->next->prev = fcgi_pmgr->prev;
        free(fcgi_pmgr);
        return;
    }
}


struct fcgi_pmgr_t *
fcgi_pmgr_match(struct htdx_t *htdx, char *extname)
{
    struct fcgi_pmgr_t *curr, *last;
    if (htdx->fcgi_pmgrs) {
		curr = htdx->fcgi_pmgrs;
		last = curr->prev;
		while (1) {
			if (strcasecmp(curr->cgiextname, extname) == 0) {
				return curr;
			}
			if (curr == last) {
				break;
			}
			curr = curr->next;
		}
    }
    return NULL;
}


int
fcgi_pmgr_conn(struct fcgi_pmgr_t *fcgi_pmgr, struct fcgi_conn_t *fcgi_conn)
{
    int bTrue = 1;
    struct sock_t *sock;
    sock = &fcgi_conn->sock;
    memcpy(&sock->rsa, &fcgi_pmgr->rsa, sizeof(struct usa_t));
    sock->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock->socket == 0) {
        chtd_cry(fcgi_pmgr->htdx, "ERROR: fcgi_pmgr_conn() -> socket() failed!%d");
        return 0;
    }
    setsockopt(sock->socket, SOL_SOCKET, SO_REUSEADDR, (void *)&bTrue, sizeof(bTrue));
    if (connect(sock->socket, &sock->rsa.u.sa, sizeof(sock->rsa.u)) == -1) {
        chtd_cry(fcgi_pmgr->htdx, "ERROR: connect() to fastcgi server (%s:%s) failed(%d)!",
                fcgi_pmgr->fcgid_addr, fcgi_pmgr->fcgid_port, sockerrno);
        closesocket(sock->socket);
        return 0;
    }
    fcgi_conn->fcgi_pmgr = fcgi_pmgr;
    return 1;
}


void *
chtd_set_fcgi(struct htdx_t *htdx, char *extname, char *sz_addr, char *sz_port, char *sz_cmdl)
{
    int port = atoi(sz_port);
    if (port > 65535 || port < 1) {
        sz_port = "9000";
    }
    if (strlen(sz_addr) == 0) {
        sz_addr = "0.0.0.0";
    }
    return fcgi_pmgr_add(htdx, extname, sz_addr, sz_port, sz_cmdl);
}
