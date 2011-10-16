
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/

#ifndef CHTD_H
#define CHTD_H


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
/* [ WIN32 */
#include <direct.h>
#include <process.h>
#include <winsock2.h>
#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH
#define sleep(n) Sleep(n)
#define strcasecmp(a,b) stricmp(a,b)
#ifdef _MSC_VER
/*   [ MSVC */
#define snprintf _snprintf
#include "dirent-w32.h"
#include "pthread_w32.c"
/*   ] MSVC */
#else
/*   [ MINGW */
#include <dirent.h>
#define PTW32_STATIC_LIB
#include "pthreadGC2.h"
/*   ] MINGW */
#endif
/* ] WIN32 */
#else
/* [ Unix/Linux */
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#define SOCKET int
/* ] */
#endif


/*
[ PCRE
*/
//#define HAVE_PCRE
#ifdef HAVE_PCRE
#define PCRE_STATIC
#include "pcre.h"
#endif
/*
]
*/


/*
[ DEBUG
*/
#ifdef DEBUG
#ifdef _MSC_VER
#define DEBUG_TRACE(x,...) chtd_cry_x(NULL, __FILE__, __LINE__, x, __VA_ARGS__)
#else
#define DEBUG_TRACE(va...) chtd_cry_x(NULL, __FILE__, __LINE__, ##va)
#endif
#else
#define DEBUG_TRACE(...)
#endif
/*
#include "htd_debug.c"
*/
/*
]
*/


#define CHTD_VERSION "0.01a"
#define BUILDTIME __DATE__" "__TIME__


#include "buff.h"
#include "base.h"
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
    int nIdelWkers;
    int lasterr;
    int n_squeue_thread;
    int n_listen_thread;
    int log_buff_size;
    char *log_file;
    char *log_buff;
    time_t birthtime;
    char *SERVER_PROTOCOL;
    char *SERVER_SOFTWARE;
    char *addr;
    char *port;
    struct sock_t sock;
    int    squeue_size;
    struct squeue_t *squeue;
    struct squeue_t *squeue_a;
    struct squeue_t *squeue_z;
    pthread_t master_tid;
    pthread_t listen_tid;
    pthread_t squeue_tid;
    pthread_mutex_t mutex;
    pthread_mutex_t mx_wk;
    pthread_cond_t  cv_wk_idel;
    int cv_wk_get_wait;
    pthread_mutex_t mx_sq;
    pthread_cond_t  cv_sq_put;
    pthread_cond_t  cv_sq_get;
    int cv_sq_put_wait;
    int cv_sq_get_wait;
    struct wker_t  *wkers;
    struct uhook_t *uhooks;
    struct vhost_t *vhosts;
    struct mime_type_t *mime_types;
    struct fcgi_pmgr_t *fcgi_pmgr;
    enum
    {
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


#define str2equ(s, c0, c1)                                                     \
  ( s[0] == c0 && s[1] == c1 )

#define str3equ(s, c0, c1, c2)                                                 \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 )

#define str4equ(s, c0, c1, c2, c3)                                             \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 )

#define str5equ(s, c0, c1, c2, c3, c4)                                         \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 )

#define str6equ(s, c0, c1, c2, c3, c4, c5)                                     \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4         \
    && s[5] == c5 )

#define str7equ(s, c0, c1, c2, c3, c4, c5, c6)                                 \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4         \
    && s[5] == c5 && s[6] == c6 )

#define str8equ(s, c0, c1, c2, c3, c4, c5, c6, c7)                             \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4         \
    && s[5] == c5 && s[6] == c6 && s[7] == c7 )

#define  strequ(str1,str2) (    strcmp(str1,str2)==0)
#define striequ(str1,str2) (strcasecmp(str1,str2)==0)


#include "log.h"
#include "mime_type.h"
#include "conn.h"
#include "reqs.h"
#include "http_status_lines.h"
#include "http_list_dir.h"
#include "http_send_file.h"
#include "http.h"
#include "uhook.h"
#include "vhost.h"
#include "squeue.h"
#include "wker.h"
#include "fcgi.h"
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
