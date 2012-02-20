
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"


int
worker_thread(struct wker_t *wker)
{
    struct htdx_t *htdx = wker->htdx;
    struct conn_t *conn;
    struct sock_t *sock;
    pthread_mutex_lock(&htdx->mutex);
    htdx->n_worker_thread++;
    pthread_mutex_unlock(&htdx->mutex);
    sock = calloc(1, sizeof(struct sock_t));
    while (1) {
        wker->status = WK_IDEL;
        if (!squeue_get(htdx, sock)) {
            if (htdx->status == CHTD_RUNNING) {
                chtd_cry(htdx, "worker_thread() -> squeue_get() failed!");
            }
            break;
        }

        wker->status = WK_BUSY;
        conn = conn_new(wker);
        conn = wker->conn;
        conn->sock = sock;

        wker->nConn++;
        conn_set_recv_timeout(conn, 20 * 1000);
        conn_set_send_timeout(conn, 20 * 1000);
        conn_parse_addr(conn);

        while (1) {
            if (conn_recv_reqs_head(conn)) {
                reqs_proc(conn);
                /* [ should keep alive? */
                if (conn->keep_alive) {
                    wker->status = WK_KEEP;
                    continue;
                } else {
                    break;
                }
                /* ] */
            } else {
                break;
            }
        }

        conn_close(conn);
        conn_del(conn);
        wker->conn = NULL;
        if (htdx->status != CHTD_RUNNING) {
            break;
        }
    }
    free(sock);
    pthread_mutex_lock(&htdx->mutex);
    htdx->n_worker_thread--;
    pthread_mutex_unlock(&htdx->mutex);
    wker->status = WK_DEAD;
    chtd_cry(htdx, "worker_thread() end!");
    pthread_exit(NULL);
    return 0;
}


int
listen_thread(struct htdx_t *htdx)
{
    /* accept loop */
    struct sock_t sock;
    struct timeval tv;
    int n;
    fd_set readfds;
    htdx->n_listen_thread = 1;
    while (htdx->status == CHTD_RUNNING) {
        tv.tv_sec  = 0;
        tv.tv_usec = 1000 * 1000;
        FD_ZERO(&readfds);
        FD_SET(htdx->sock.socket, &readfds);
        n = select(htdx->sock.socket + 1, &readfds, NULL, NULL, &tv);
        if (n > 0) {
            if (FD_ISSET(htdx->sock.socket, &readfds)) {
                sock.rsa.len = sizeof(sock.rsa.u);
                sock.socket = accept(htdx->sock.socket, &sock.rsa.u.sa, (void *)&sock.rsa.len);
                if (sock.socket <= 0) {
                    htdx->status = CHTD_SUSPEND;
                    chtd_cry(htdx, "accept() return 0 OR -1!");
                    break;
                }
                sock.lsa.len = sizeof(sock.lsa.u);
                getsockname(sock.socket, &sock.lsa.u.sa, (void *)&sock.lsa.len);
                if (!squeue_put(htdx, &sock)) {
                    chtd_cry(htdx, "squeue_put() return failed!");
                    break;
                }
            }
        } else
        if (n < 0) {
            chtd_cry(htdx, "select() return SOCKET_ERROR!");
            htdx->status = CHTD_SUSPEND;
        } else {
            /* select() timeout */
        }
    }
    htdx->n_listen_thread = 0;
    chtd_cry(htdx, "listen_thread() end!");
    return 0;
}


int
master_thread(struct htdx_t *htdx)
{
    /* [ Init */
#ifdef _WIN32
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 0), &wsadata)) {
        chtd_cry(htdx, "WSAStartup() failed!");
        return 0;
    }
#endif

    htdx->status = CHTD_STARTUP;

    init_squeue(htdx);
    init_wkers (htdx);
    /* ] */

    /* [ Startup */
    do {
        int bTrue = 1;
        struct usa_t *lsa;

        /* socket() */
        htdx->sock.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (htdx->sock.socket == -1) {
            chtd_cry(htdx, "master_thread() -> socket() error!");
            htdx->status = CHTD_SUSPEND;
            break;
        }

        setsockopt(htdx->sock.socket, SOL_SOCKET, SO_REUSEADDR, (void *)&bTrue, sizeof(bTrue));
        setsockopt(htdx->sock.socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&bTrue, sizeof(bTrue));

        /* bind() */
        lsa = &htdx->sock.lsa;
        lsa->u.sin.sin_family = AF_INET;
        lsa->u.sin.sin_addr.s_addr = inet_addr(htdx->addr);
        lsa->u.sin.sin_port = htons(atoi(htdx->port));
        lsa->len = sizeof(lsa->u);
        if (bind(htdx->sock.socket, &lsa->u.sa, lsa->len) == -1) {
            chtd_cry(htdx, "bind() to %s:%s error!", htdx->addr, htdx->port);
            htdx->status = CHTD_SUSPEND;
            break;
        }

        /* listen() */
        if (listen(htdx->sock.socket, SOMAXCONN) == -1) {
            chtd_cry(htdx, "listen() on %s:%s error!", htdx->addr, htdx->port);
            htdx->status = CHTD_SUSPEND;
            break;
        }

        htdx->status = CHTD_RUNNING;

        /* listen_thread() */
        if (pthread_create(&htdx->listen_tid, NULL, (void *)listen_thread, htdx) != 0) {
            chtd_cry(htdx, "create listen_thread falied!");
            htdx->status = CHTD_SUSPEND;
            break;
        }

    } while (0);
    /* ] */

    if (htdx->status == CHTD_RUNNING) {
        htdx->birthtime = time(NULL);
        while (htdx->status == CHTD_RUNNING) {
            wker_stat(htdx);
            sleep(100);
        }
    }

    /* [ Shutdown */
    if (htdx->sock.socket > 0) {
#ifdef _WIN32
        closesocket(htdx->sock.socket);
#else
        close(htdx->sock.socket);
#endif
    }

    /* broadcast all worker_thread to exit */
    pthread_cond_broadcast(&htdx->cv_sq_put);

    /* wait until listen_thread() & worker_thread() exit */
    while (htdx->n_listen_thread
        || htdx->n_worker_thread) {
        sleep(100);
    }
    /* ] */

    /* [ Destroy */
    free_wkers (htdx);
    free_squeue(htdx);
#ifdef _WIN32
    WSACleanup();
#endif
    /* ] */
    chtd_cry(htdx, "master_thread() end!");
    htdx->status = CHTD_STOPPED;
    return 0;
}


struct htdx_t *
chtd_create(void) {
    static char SERVER_SOFTWARE[256];
    struct htdx_t *htdx;
    htdx = calloc(1, sizeof(struct htdx_t));
    htdx->addr                  = strdup("0.0.0.0");
    htdx->port                  = strdup("8080");
    snprintf(SERVER_SOFTWARE,
        sizeof(SERVER_SOFTWARE),
#ifdef DEBUG
        "Cutehttpd/%s (Built: Debug; %s; %s/%d; %08x%2x;)",
#else
        "Cutehttpd/%s (Built: %s; %s/%d; %08x%2x;)",
#endif
        CHTD_VERSION,
        BUILDTIME,
        CPER,
        CVER,
        REV_A,
        REV_B >> 24);
    htdx->SERVER_SOFTWARE       = SERVER_SOFTWARE;
    htdx->SERVER_PROTOCOL       = "HTTP/1.1";
    htdx->max_workers           = 32;
    htdx->squeue_size           = 1024;
    htdx->keep_alive_timeout    = 0;
    htdx->max_post_size         = 8 * 1024 * 1024;
#ifdef PTW32_STATIC_LIB
    pthread_win32_process_attach_np();
    pthread_win32_thread_attach_np();
#endif
    pthread_mutex_init(&htdx->mutex, NULL);
    return htdx;
}


int
chtd_delete(struct htdx_t *htdx)
{
    if (htdx->status == CHTD_STOPPED) {
        free_vhosts(htdx);
        free_uhooks(htdx);
        free(htdx->addr);
        free(htdx->port);
        pthread_mutex_destroy(&htdx->mutex);
#ifdef PTW32_STATIC_LIB
        pthread_win32_thread_detach_np();
        pthread_win32_process_detach_np();
#endif
        return 0;
    }
    return -1;
}


int
chtd_start(struct htdx_t *htdx)
{
    pthread_t ptid;
    htdx->status = CHTD_STARTUP;
    /* master_thread */
    if (pthread_create(&ptid, NULL, (void *)master_thread, htdx) != 0) {
        chtd_cry(htdx, "create master_thread falied!");
        htdx->status = CHTD_STOPPED;
        return -1;
    }
    return 0;
}


int
chtd_stop(struct htdx_t *htdx)
{
    if (htdx->status == CHTD_RUNNING) {
        htdx->status = CHTD_SUSPEND;
        return 0;
    }
    return 1;
}


int
chtd_get_status(struct htdx_t *htdx)
{
    return htdx->status;
}


int
chtd_set_opt(struct htdx_t *htdx, char *opt, char *value)
{
    int n = atoi(value);

    /* port */
    if (0 == strcasecmp(opt, "port")) {
        if (n > 65535 || n < 1) {
            return 0;
        }
        free(htdx->port);
        htdx->port = strdup(value);
        return 1;
    }

    /* addr */
    if (0 == strcasecmp(opt, "addr")) {
        free(htdx->addr);
        htdx->addr = strdup(value);
        return 1;
    }

    /* max_workers */
    if (0 == strcasecmp(opt, "max_workers")) {
        if (n > 1000) {
            n = 1000;
        }
        if (n < 1) {
            n = 1;
        }
        htdx->max_workers = n;
        return 1;
    }

    /* keep_alive */
    if (0 == strcasecmp(opt, "keep_alive")) {
        if (n > 60) {
            n = 60;
        }
        if (n < 1) {
            n = 1;
        }
        htdx->keep_alive_timeout = n;
        return 1;
    }

    /* max_post_size */
    if (0 == strcasecmp(opt, "max_post_size")) {
        if (n <= 0) {
            return 0;
        }
        htdx->max_post_size = n*1024*1024;
        return 1;
    }
    return 0;
}
