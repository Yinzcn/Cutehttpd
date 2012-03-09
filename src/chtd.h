
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/

#ifndef CHTD_H
#define CHTD_H


#include "config.h"
#include "version.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef _WIN32
/* [ WIN32 */

    #include <winsock2.h>
    #include <direct.h>
    #include <process.h>
    #include "pthread_w32.c"

    #ifdef HAVE_DIRENT
        #include <dirent.h>
    #else
        #include "dirent_w32.h"
    #endif

/* ] WIN32 */
#else
/* [ Linux */

    #include <sys/select.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <pthread.h>
    #define HAVE_STRNDUP
    #define HAVE_REALPATH

/* ] */
#endif


/* [ PCRE */
#ifdef HAVE_PCRE
    #define PCRE_STATIC
    #include "pcre.h"
#endif
/* ] */


/* [ DEBUG */
#ifdef DEBUG
    #define DEBUG_TRACE(...) chtd_cry_x(__FILE__, __LINE__, NULL, __VA_ARGS__)
#else
    #define DEBUG_TRACE(...)
#endif
/*
#include "htd_debug.c"
*/
/* ] */


#include "base.h"
#include "buff.h"
#include "sock.h"
#include "namevalue.h"
#include "parse_header.h"


/*
  Cutehttpd context
*/
struct htdx_t
{
    int keep_alive_timeout;
    int max_post_size;
    int max_workers;
    int nConn;
    int nReqs;
    int nBadReqs;
    int lasterr;
    int n_listen_thread;
    int n_worker_thread;
    char *log_file;
    char *log_buff;
    time_t birthtime;
    char *SERVER_PROTOCOL;
    char *SERVER_SOFTWARE;
    char *addr;
    char *port;
    struct sock_t sock;

    /* [ squeue */
    int    squeue_size;
    struct squeue_t *squeue;
    struct squeue_t *squeue_a;
    struct squeue_t *squeue_z;
    pthread_mutex_t mx_sq;
    pthread_cond_t  cv_sq_put;
    pthread_cond_t  cv_sq_get;
    /* ] */

    pthread_t master_tid;
    pthread_t listen_tid;
    pthread_mutex_t mutex;
    struct wker_t  *wkers;
    struct uhook_t *uhooks;
    struct vhost_t *vhosts;
    struct mime_type_t *mime_types;
    #ifdef CHTD_FCGI
    struct fcgi_pmgr_t *fcgi_pmgrs;
    #endif
    enum {
        CHTD_STOPPED = 0,
        CHTD_STARTUP,
        CHTD_SUSPEND,
        CHTD_RUNNING
    } status;
};


#define CR '\r'
#define LF '\n'
#define HT '\t'
#define SP ' '

#define CHTD_OK 0
#define CHTD_ERROR -1


#include "log.h"
#include "mime_type.h"
#include "conn.h"
#include "reqs.h"
#include "http_status_lines.h"
#include "http_list_dir.h"
#ifdef CHTD_FCGI
    #include "../ext/fcgi.h"
    #include "../ext/fcgi_pmgr.h"
#endif
#include "http_send_file.h"
#include "http.h"
#include "uhook.h"
#include "vhost.h"
#include "squeue.h"
#include "wker.h"
#include "status_info.h"
#include "test_ext.h"
#include "debug.h"


int
worker_thread(struct wker_t *);

int
squeue_thread(struct htdx_t *);

int
listen_thread(struct htdx_t *);

int
master_thread(struct htdx_t *);

struct htdx_t *
chtd_create(void);

int
chtd_delete(struct htdx_t *);

int
chtd_start(struct htdx_t *);

int
chtd_stop(struct htdx_t *);

int
chtd_get_status(struct htdx_t *);

int
chtd_set_opt(struct htdx_t *, char *, char *);


#endif
