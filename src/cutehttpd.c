
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"


int
worker_thread(struct wker_t *wker)
{
    chtd_cry(wker->htdx, "worker_thread()!");
    while (1)
    {
        pthread_mutex_lock(&wker->mx_wake);
        chtd_cry(wker->htdx, "pthread_mutex_lock() done!");
        if (wker->birthtime == 0)
        {
            wker->birthtime = time(NULL);
        } else {
            pthread_cond_wait(&wker->cv_wake, &wker->mx_wake);
        }
        chtd_cry(wker->htdx, "pthread_cond_wait() done!");

        wker->status = WK_BUSY;

        struct htdx_t *htdx = wker->htdx;
        struct conn_t *conn = wker->conn;

        wker->nConn++;
        conn_parse_addr(conn);

        while (1)
        {
            if (conn_recv_reqs_strs(conn))
            {
                reqs_proc(conn);
                /* [ should keep alive? */
                if (conn->keep_alive)
                {
                    wker->status = WK_KEEP;
                    continue;
                }
                else
                {
                    break;
                }
                /* ] */
            }
            else
            {
                break;
            }
        }

        conn_close(conn);
        conn_del(conn);
        wker->conn = NULL;
        put_wait_wker(wker);
        pthread_mutex_unlock(&wker->mx_wake);
        if (htdx->status != CHTD_RUNNING)
        {
            break;
        }
    }
    wker->birthtime = 0;
    put_idel_wker(wker);
    return 0;
}


int
squeue_thread(struct htdx_t *htdx)
{
    htdx->n_squeue_thread = 1;
    struct wker_t *wker;
    while (htdx->status == CHTD_RUNNING)
    {
        wker = get_idel_wker(htdx);
        if (!wker)
        {
            chtd_cry(htdx, "squeue_thread() -> get_idel_wker() failed!");
            sleep(100);
            continue;
        }
        chtd_cry(htdx, "get_idel_wker() done!");
        wker->conn = conn_new(wker);
        if (!squeue_get(htdx, &wker->conn->sock))
        {
            conn_del(wker->conn);
            wker->conn = NULL;
            put_idel_wker(wker);
            break;
        }
        chtd_cry(htdx, "squeue_get() done!");
        wker_wake(wker);
        chtd_cry(htdx, "wker_wake(2) done!");
    }
    htdx->n_squeue_thread = 0;
    return 0;
}


int
listen_thread(struct htdx_t *htdx)
{
    htdx->n_listen_thread = 1;
    /* accept loop */
    struct sock_t sock;
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 1000;
    fd_set readfds;
    int n;
    while (htdx->status == CHTD_RUNNING)
    {
        FD_ZERO(&readfds);
        FD_SET (htdx->sock.socket, &readfds);
        n = select(htdx->sock.socket + 1, &readfds, NULL, NULL, &tv);
        if (n > 0)
        {
            if (FD_ISSET(htdx->sock.socket, &readfds))
            {
                memset(&sock, 0, sizeof(struct sock_t));
                sock.rsa.len = sizeof(sock.rsa.u);
                sock.socket  = accept(htdx->sock.socket, &sock.rsa.u.sa, &sock.rsa.len);
                /* [ accept() error? */
                if (sock.socket == -1)
                {
                    htdx->status = CHTD_SUSPEND;
                    chtd_cry(htdx, "accept() return -1!");
                    break;
                }
                /* ] */
                sock.lsa.len = sizeof(sock.lsa.u);
                getsockname(sock.socket, &sock.lsa.u.sa, &sock.lsa.len);
                if (!squeue_put(htdx, &sock))
                {
                    break;
                }
                chtd_cry(htdx, "squeue_put() done!");
            }
        }
        else if (n < 0)
        {
            chtd_cry(htdx, "select() return SOCKET_ERROR!");
            htdx->status = CHTD_SUSPEND;
        }
        else
        {
            /* select() timeout */
            ;
        }
    }
    htdx->n_listen_thread = 0;
    return 0;
}


int
master_thread(struct htdx_t *htdx)
{
    htdx->status = CHTD_STARTUP;
    /* [ Init */
    #ifdef WIN32
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 0), &wsadata))
    {
        chtd_cry(htdx, "WSAStartup() failed!");
        return 0;
    }
    #endif
    #ifdef PTW32_STATIC_LIB
    pthread_win32_process_attach_np();
    pthread_win32_thread_attach_np ();
    #endif
    pthread_mutex_init  (&htdx->mutex,      NULL);
    pthread_mutex_init  (&htdx->mx_sq,      NULL);
    pthread_cond_init   (&htdx->cv_sq_get,  NULL);
    pthread_cond_init   (&htdx->cv_sq_put,  NULL);
    pthread_mutex_init  (&htdx->mx_wk,      NULL);
    pthread_cond_init   (&htdx->cv_wk_idel, NULL);
    init_squeue         (htdx);
    init_wkers          (htdx);
    /* ] */

    /* [ Startup */
    do {
        /* socket() */
        htdx->sock.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (htdx->sock.socket == -1)
        {
            chtd_cry(htdx, "master_thread() -> socket() error!");
            htdx->status = CHTD_STOPPED;
            break;
        }

        int bTrue = 1;
        setsockopt(htdx->sock.socket, SOL_SOCKET, SO_REUSEADDR, &bTrue, sizeof(bTrue));

        /* bind() */
        struct usa_t *lsa = &htdx->sock.lsa;
        lsa->u.sin.sin_family = AF_INET;
        lsa->u.sin.sin_addr.s_addr = inet_addr(htdx->addr);
        lsa->u.sin.sin_port = htons(atoi(htdx->port));
        lsa->len = sizeof(lsa->u);
        if (bind(htdx->sock.socket, &lsa->u.sa, lsa->len) == -1)
        {
            chtd_cry(htdx, "bind() to %s:%s error!", htdx->addr, htdx->port);
            htdx->status = CHTD_STOPPED;
            break;
        }

        /* listen() */
        if (listen(htdx->sock.socket, SOMAXCONN) == -1)
        {
            chtd_cry(htdx, "listen() on %s:%s error!", htdx->addr, htdx->port);
            htdx->status = CHTD_STOPPED;
            break;
        }

        htdx->status = CHTD_RUNNING;

        /* listen_thread */
        if (pthread_create(&htdx->listen_tid, NULL, (void *)listen_thread, htdx) != 0)
        {
            chtd_cry(htdx, "create listen_thread falied!");
            htdx->status = CHTD_STOPPED;
            break;
        }

        /* squeue_thread */
        if (pthread_create(&htdx->squeue_tid, NULL, (void *)squeue_thread, htdx) != 0)
        {
            chtd_cry(htdx, "create squeue_thread falied!");
            htdx->status = CHTD_STOPPED;
            break;
        }

    } while (0);
    /* ] */


    if (htdx->status == CHTD_RUNNING)
    {
        htdx->birthtime = time(NULL);
        while (htdx->status == CHTD_RUNNING)
        {
            wker_stat(htdx);
            sleep(100);
        }
    }

    /* [ Shutdown */
    if (htdx->sock.socket > 0)
    {
        #ifdef WIN32
        closesocket(htdx->sock.socket);
        #else
        close(htdx->sock.socket);
        #endif
    }

    if (htdx->cv_sq_put_wait)
    {
        pthread_cond_signal(&htdx->cv_sq_put);
    }

    if (htdx->cv_sq_get_wait)
    {
        pthread_cond_signal(&htdx->cv_sq_get);
    }

    while (htdx->n_listen_thread)
    {
        sleep(100);
    }

    while (htdx->n_squeue_thread)
    {
        sleep(100);
    }

    while (htdx->nIdelWkers != htdx->max_workers)
    {
        sleep(100);
    }
    /* ] */

    /* [ Destroy */
    pthread_mutex_destroy (&htdx->mutex);
    pthread_mutex_destroy (&htdx->mx_sq);
    pthread_cond_destroy  (&htdx->cv_sq_get);
    pthread_cond_destroy  (&htdx->cv_sq_put);
    #ifdef PTW32_STATIC_LIB
    pthread_win32_thread_detach_np ();
    pthread_win32_process_detach_np();
    free_wkers  (htdx);
    free_squeue (htdx);
    #endif
    #ifdef WIN32
    WSACleanup();
    #endif
    /* ] */
    htdx->status = CHTD_STOPPED;
    return 0;
}


/*
  API
*/
struct htdx_t *
chtd_create()
{
    struct htdx_t *htdx;
    htdx = calloc(1, sizeof(struct htdx_t));
    htdx->addr                  = strdup("0.0.0.0");
    htdx->port                  = strdup("8080");
    htdx->SERVER_SOFTWARE       = "Cutehttpd/"CHTD_VERSION" (Built: "BUILDTIME")";
    htdx->max_workers           = 32;
    htdx->squeue_size           = 1024;
    htdx->keep_alive_timeout    = 0;
    htdx->max_post_size         = 8*1024*1024;
    return htdx;
}


/*
  API
*/
int
chtd_delete(struct htdx_t *htdx)
{
    if (htdx->status == CHTD_STOPPED)
    {
        free_vhosts (htdx);
        free_uhooks (htdx);
        free  (htdx->addr);
        free  (htdx->port);
        return 1;
    }
    return 0;
}


/*
  API
*/
int
chtd_start(struct htdx_t *htdx)
{
    htdx->status = CHTD_STARTUP;
    /* master_thread */
    if (pthread_create(&htdx->master_tid, NULL, (void *)master_thread, htdx) != 0)
    {
        chtd_cry(htdx, "create master_thread falied!");
        htdx->status = CHTD_STOPPED;
        return -1;
    }
    return 0;
}


/*
  API
*/
int
chtd_stop(struct htdx_t *htdx)
{
    if (htdx->status == CHTD_RUNNING)
    {
        htdx->status = CHTD_SUSPEND;
        return 0;
    }
    return 1;
}


/*
  API
*/
int
chtd_get_status(struct htdx_t *htdx)
{
    return htdx->status;
}


/*
  API
*/
int
chtd_set_opt(struct htdx_t *htdx, char *opt, char *value)
{
    int n;
    n = atoi(value);

    /* port */
    if (0 == strcasecmp(opt, "port"))
    {
        if (n > 65535 || n < 1)
        {
            return 0;
        }
        free(htdx->port);
        htdx->port = strdup(value);
        return 1;
    }

    /* addr */
    if (0 == strcasecmp(opt, "addr"))
    {
        free(htdx->addr);
        htdx->addr = strdup(value);
        return 1;
    }

    /* max_workers */
    if (0 == strcasecmp(opt, "max_workers"))
    {
        if (n > 1000)
        {
            n = 1000;
        }
        if (n <    1)
        {
            n =    1;
        }
        htdx->max_workers = n;
        return 1;
    }

    /* keep_alive */
    if (0 == strcasecmp(opt, "keep_alive"))
    {
        if (n > 60)
        {
            n = 60;
        }
        if (n <  1)
        {
            n =  1;
        }
        htdx->keep_alive_timeout = n;
        return 1;
    }

    /* max_post_size */
    if (0 == strcasecmp(opt, "max_post_size"))
    {
        if (n <= 0)
        {
            return 0;
        }
        htdx->max_post_size = n*1024*1024;
        return 1;
    }
    return 0;
}
