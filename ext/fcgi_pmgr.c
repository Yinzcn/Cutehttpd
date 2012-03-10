
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
    fcgi_pmgr->n_conn_max = 64;
    strncpy(fcgi_pmgr->cgiextname, extname, 15);
    strncpy(fcgi_pmgr->fcgid_addr, sz_addr, 63);
    strncpy(fcgi_pmgr->fcgid_port, sz_port, 15);
    strncpy(fcgi_pmgr->fcgid_cmdl, sz_cmdl, 255);
    pthread_mutex_init(&fcgi_pmgr->mutex, NULL);
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
    if (!fcgi_pmgr) {
        return;
    }
    pthread_mutex_destroy(&fcgi_pmgr->mutex);
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
    if (htdx->fcgi_pmgrs == NULL) {
        return NULL;
    }
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
    return NULL;
}


struct fcgi_proc_t *
fcgi_pmgr_proc_new(struct fcgi_pmgr_t *fcgi_pmgr)
{
    struct fcgi_proc_t *fcgi_proc;
    char tmp1[8], *tmp2, *cmdl;
    struct fcgi_proc_t *curr, *last;
    int port = atoi(fcgi_pmgr->fcgid_port);
    curr = fcgi_pmgr->fcgi_procs;
    if (curr) {
        last = curr->prev;
        while (1) {
            if (curr->port == port) {
                port++;
                curr = last->next;
            }
            if (curr == last) {
                break;
            }
            curr = curr->next;
        }
    }
    tmp2 = str_replace("@addr@", fcgi_pmgr->fcgid_addr, fcgi_pmgr->fcgid_cmdl);
    cmdl = str_replace("@port@", itoa(port, tmp1, 10), tmp2);
    free(tmp2);
    fcgi_proc = calloc(1, sizeof(struct fcgi_proc_t));
    if (fcgi_proc) {
        fcgi_proc->rsa.u.sin.sin_family = AF_INET;
        fcgi_proc->rsa.u.sin.sin_addr.s_addr = inet_addr(fcgi_pmgr->fcgid_addr);
        fcgi_proc->rsa.u.sin.sin_port = htons(port);
        fcgi_proc->n_conn_max = 5;
        fcgi_proc->port = port;
        fcgi_proc->cmdl = cmdl;
        if (fcgi_pmgr->fcgi_procs) {
            fcgi_proc->prev = fcgi_pmgr->fcgi_procs->prev;
            fcgi_proc->next = fcgi_pmgr->fcgi_procs;
            fcgi_proc->prev->next = fcgi_proc;
            fcgi_proc->next->prev = fcgi_proc;
        } else {
            fcgi_proc->prev = fcgi_proc;
            fcgi_proc->next = fcgi_proc;
            fcgi_pmgr->fcgi_procs = fcgi_proc;
        }
    }
    return fcgi_proc;
}


struct fcgi_proc_t *
fcgi_pmgr_proc_spawn(struct fcgi_proc_t *fcgi_proc)
{
    PROCESS_INFORMATION pi;
    STARTUPINFOA si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    struct envblk_t envblk = {};
    envblk_add(&envblk, "SYSTEMDRIVE", getenv("SYSTEMDRIVE"));
    envblk_add(&envblk, "SYSTEMROOT", getenv("SYSTEMROOT"));
    envblk_add(&envblk, "WINDIR", getenv("WINDIR"));
    envblk_add(&envblk, "TEMP", getenv("TEMP"));
    envblk_add(&envblk, "PATH", getenv("PATH"));
    envblk_add(&envblk, "PHP_FCGI_MAX_REQUESTS", "5");
    if (CreateProcess(NULL,
            fcgi_proc->cmdl,
            NULL,
            NULL,
            TRUE,
            CREATE_NEW_PROCESS_GROUP,
            envblk.blkbuf,
            NULL,//dir,
            &si,
            &pi)) {
    } else {
        chtd_cry(fcgi_pmgr->htdx, "CreateProcess() failed! [%s]", fcgi_proc->cmdl);
    }
    DEBUG_TRACE("fcgi_pmgr_proc_spawn() -> %s", fcgi_proc->cmdl);
    return fcgi_proc;
}


struct fcgi_proc_t *
fcgi_pmgr_proc_assign(struct fcgi_pmgr_t *fcgi_pmgr)
{
    struct fcgi_proc_t *curr, *last, *proc = NULL;
    curr = fcgi_pmgr->fcgi_procs;
    if (curr) {
        last = curr->prev;
        while (1) {
            DEBUG_TRACE("fcgi_pmgr_proc_spawn() -> %d, %d", curr->n_conn_max, curr->n_conn_cur);
            if (curr->n_conn_cur < curr->n_conn_max) {
                proc = curr;
                break;
            } else {
                
            }
            if (curr == last) {
                break;
            }
            curr = curr->next;
        }
    }
    if (proc == NULL) {
        proc = fcgi_pmgr_proc_new(fcgi_pmgr);
        fcgi_pmgr_proc_spawn(proc);
    }
    proc->n_conn_cur++;
    return proc;
}


int
fcgi_pmgr_conn(struct fcgi_pmgr_t *fcgi_pmgr, struct fcgi_conn_t *fcgi_conn)
{
    struct sock_t *sock;
    struct fcgi_proc_t *fcgi_proc;
    fcgi_proc = fcgi_pmgr_proc_assign(fcgi_pmgr);
    if (!fcgi_proc) {
        return 0;
    }
    memcpy(&fcgi_conn->sock.rsa, &fcgi_proc->rsa, sizeof(struct usa_t));
    sock = &fcgi_conn->sock;
    sock->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock->socket == 0) {
        chtd_cry(fcgi_pmgr->htdx, "ERROR: fcgi_pmgr_conn() -> socket() failed!%d");
        return -1;
    }
    if (connect(sock->socket, &sock->rsa.u.sa, sizeof(sock->rsa.u)) == -1) {
        chtd_cry(fcgi_pmgr->htdx, "ERROR: connect() to fastcgi server (%s:%s) failed!",
            fcgi_pmgr->fcgid_addr, fcgi_pmgr->fcgid_port);
        closesocket(sock->socket);
        return -1;
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
