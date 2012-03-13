
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#include <winsock2.h>


#include "../src/pthread_w32.c"
#include "buff.c"


#ifdef _WIN32
    #define x_msleep(ms) Sleep(ms)
    #define strcasecmp(a,b) stricmp(a,b)
    #define snprintf _snprintf
#else
    #define x_msleep(ms) usleep(ms * 1000)
#endif


#ifdef _WIN32
    #define SHUT_RD   SD_RECEIVE
    #define SHUT_WR   SD_SEND
    #define SHUT_RDWR SD_BOTH
    #define sockerrno WSAGetLastError()
#else
    #define closesocket(s) close(s)
    #define SOCKET int
    #define sockerrno errno
#endif


struct usa_t   /* unified socket address */
{
    int len;
    union {
        struct sockaddr sa;
        struct sockaddr_in sin;
    } u;
};


struct sock_t
{
    SOCKET socket;
    struct usa_t lsa;
    struct usa_t rsa;
};


struct srvx_t
{
    int n_worker_max;
    int n_worker_cur;
    char *log_file;
    char *log_buff;
    char *f_addr, *d_addr;
    char *f_port, *d_port;
    struct sock_t sock;
    struct usa_t drsa;
    pthread_t master_tid;
    pthread_t listen_tid;
    pthread_mutex_t server_mutex;
    pthread_mutex_t srvlog_mutex;
    pthread_mutex_t accept_mutex;
    enum {
        SRVX_STOPPED = 0,
        SRVX_STARTUP,
        SRVX_SUSPEND,
        SRVX_RUNNING
    } status;
};


char *
x_nowstr(void)
{
    static char buff[20];
    time_t rawtime = time(NULL);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&rawtime));
    return buff;
}


int
file_add(char *filename, char *data, int size)
{
    FILE *pFile;
    if (!filename || !data) {
        return 0;
    }
    if (!size) {
        size = strlen(data);
    }
    pFile = fopen(filename, "ab");
    if (!pFile) {
        return 0;
    }
    fseek(pFile, 0, SEEK_END);
    fwrite(data, 1, size, pFile);
    fclose(pFile);
    return 1;
}


void
srvx_cry(struct srvx_t *srvx, char *f, ...)
{
    char b[1024] = { 0 };
    int n;
    va_list a;
    n = snprintf(b, sizeof(b), "[%s] ", x_nowstr());
    va_start(a, f);
    n += vsnprintf(b + n, sizeof(b) - n, f, a);
    va_end(a);
    strcat(b, "\r\n");
    pthread_mutex_lock  (&srvx->srvlog_mutex);
    printf("%s", b);
    file_add("portpipe.log", b, strlen(b));
    pthread_mutex_unlock(&srvx->srvlog_mutex);
}


void
sock_non_blocking(struct sock_t *sock)
{
#ifdef _WIN32
    static unsigned long u = 1;
    ioctlsocket(sock->socket, FIONBIO, &u);
#else
    (void)fcntl(sock->socket, F_SETFL, fcntl(sock->socket, F_GETFL, 0) | O_NONBLOCK);
#endif
}


void
sock_close(struct sock_t *sock)
{
    if (sock == NULL) {
        return;
    }
    if (sock->socket > 0) {
        static char buff[256];
        static int o = 1;
        setsockopt(sock->socket, SOL_SOCKET, SO_DONTLINGER, (void *)&o, sizeof(o));
        shutdown(sock->socket, SHUT_WR);
        sock_non_blocking(sock);
        while (recv(sock->socket, buff, 256, 0) > 0);
        closesocket(sock->socket);
        sock->socket = 0;
    }
}


int
worker_thread(struct srvx_t *srvx)
{
    int wid;
    static int bTrue = 1;
    struct sock_t f_sock, d_sock;
    pthread_mutex_lock  (&srvx->server_mutex);
    wid = srvx->n_worker_cur++;
    pthread_mutex_unlock(&srvx->server_mutex);
    f_sock.rsa.len = sizeof(f_sock.rsa.u);
    while (srvx->status == SRVX_RUNNING) {
        pthread_mutex_lock(&srvx->accept_mutex);
        f_sock.socket = accept(srvx->sock.socket, &f_sock.rsa.u.sa, (void *)&f_sock.rsa.len);
        pthread_mutex_unlock(&srvx->accept_mutex);
        if (f_sock.socket <= 0) {
            srvx_cry(srvx, "accept() got %d!", sockerrno);
            continue;
        }
        d_sock.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        setsockopt(d_sock.socket, SOL_SOCKET, SO_REUSEADDR, (void *)&bTrue, sizeof(bTrue));
        if (connect(d_sock.socket, &srvx->drsa.u.sa, sizeof(srvx->drsa.u)) != 0) {
            srvx_cry(srvx, "connect() got %d!", sockerrno);
        } else {
            int n;
            enum { N, S, T } stat = N;
            struct timeval tv;
            fd_set readfds;
            char text[64];
            struct bufx_t *bufx;
            bufx = bufx_new(1024, 1024 * 1024 * 32);
            bufx_put(bufx, text, snprintf(text, sizeof(text), "\n#%03d [[[\n", wid));
            while (srvx->status == SRVX_RUNNING
                && f_sock.socket > 0
                && d_sock.socket > 0)
            {
                tv.tv_sec  = 0;
                tv.tv_usec = 1000 * 1000;
                FD_ZERO(&readfds);
                FD_SET(f_sock.socket, &readfds);
                FD_SET(d_sock.socket, &readfds);
                n = select((f_sock.socket > d_sock.socket ? f_sock.socket : d_sock.socket) + 1, &readfds, NULL, NULL, &tv);
                if (n > 0) {
                    int retn;
                    char buff[1024];
                    char *pstr;
                    if (FD_ISSET(f_sock.socket, &readfds)) {
                        retn = recv(f_sock.socket, buff, 1024, 0);
                        if (retn <= 0) {
                            break;
                        }
                        if (stat != S) {
                            bufx_put(bufx, text, snprintf(text, sizeof(text), "\n#%03d s -> t\n", wid));
                            stat = S;
                        }
                        bufx_put(bufx, buff, retn);
                        retn = send(d_sock.socket, buff, retn, 0);
                        if (retn <= 0) {
                            break;
                        }
                    }
                    if (FD_ISSET(d_sock.socket, &readfds)) {
                        retn = recv(d_sock.socket, buff, 1024, 0);
                        if (retn <= 0) {
                            break;
                        }
                        if (stat != T) {
                            bufx_put(bufx, text, snprintf(text, sizeof(text), "\n#%03d t -> s\n", wid));
                            stat = T;
                        }
                        bufx_put(bufx, buff, retn);
                        retn = send(f_sock.socket, buff, retn, 0);
                        if (retn <= 0) {
                            break;
                        }
                    }
                } else
                if (n < 0) {
                    srvx_cry(srvx, "select() return SOCKET_ERROR!");
                    srvx->status = SRVX_SUSPEND;
                } else {
                    /* select() timeout */
                }
            }
            bufx_put(bufx, text, snprintf(text, sizeof(text), "\n#%03d ]]]\n", wid));
            file_add("pipe.log", bufx_link(bufx), bufx_get_used(bufx));
            bufx_del(bufx);
        }
        sock_close(&f_sock);
        sock_close(&d_sock);
    }
    pthread_mutex_lock  (&srvx->server_mutex);
    srvx->n_worker_cur--;
    pthread_mutex_unlock(&srvx->server_mutex);
    pthread_exit(NULL);
    return 0;
}


int
master_thread(struct srvx_t *srvx)
{
    /* [ Init */
    #ifdef _WIN32
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 0), &wsadata)) {
        srvx_cry(srvx, "WSAStartup() failed!");
        return 0;
    }
    #endif
    pthread_mutex_init(&srvx->server_mutex, NULL);
    pthread_mutex_init(&srvx->srvlog_mutex, NULL);
    pthread_mutex_init(&srvx->accept_mutex, NULL);
    srvx->status = SRVX_STARTUP;
    /* ] */

    srvx->drsa.u.sin.sin_family = AF_INET;
    srvx->drsa.u.sin.sin_addr.s_addr = inet_addr(srvx->d_addr);
    srvx->drsa.u.sin.sin_port = htons(atoi(srvx->d_port));

    /* [ Startup */
    do {
        int i;
        static int bTrue = 1;
        struct usa_t *lsa;

        /* socket() */
        srvx->sock.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (srvx->sock.socket == -1) {
            srvx_cry(srvx, "master_thread() -> socket() error!");
            srvx->status = SRVX_SUSPEND;
            break;
        }

        setsockopt(srvx->sock.socket, SOL_SOCKET, SO_REUSEADDR, (void *)&bTrue, sizeof(bTrue));
        setsockopt(srvx->sock.socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&bTrue, sizeof(bTrue));

        /* bind() */
        lsa = &srvx->sock.lsa;
        lsa->u.sin.sin_family = AF_INET;
        lsa->u.sin.sin_addr.s_addr = inet_addr(srvx->f_addr);
        lsa->u.sin.sin_port = htons(atoi(srvx->f_port));
        lsa->len = sizeof(lsa->u);
        if (bind(srvx->sock.socket, &lsa->u.sa, lsa->len) == -1) {
            srvx_cry(srvx, "bind() to %s:%s error(%d)!", srvx->f_addr, srvx->f_port, sockerrno);
            srvx->status = SRVX_SUSPEND;
            break;
        }

        /* listen() */
        if (listen(srvx->sock.socket, SOMAXCONN) == -1) {
            srvx_cry(srvx, "listen() on %s:%s error!", srvx->f_addr, srvx->f_port);
            srvx->status = SRVX_SUSPEND;
            break;
        }

        srvx->status = SRVX_RUNNING;

        for (i = 0; i < srvx->n_worker_max; i++) {
            int maxtry = 10;
            pthread_t t_id;
            while (maxtry--) {
                if (pthread_create(&t_id, NULL, (void *)worker_thread, srvx) == 0) {
                    break;
                }
                x_msleep(100);
            }
        }

    } while (0);
    /* ] */

    if (srvx->status == SRVX_RUNNING) {
        while (srvx->status == SRVX_RUNNING) {
            x_msleep(100);
        }
    }

    if (srvx->sock.socket > 0) {
        closesocket(srvx->sock.socket);
    }

    while (srvx->n_worker_cur) {
        x_msleep(100);
    }

    srvx_cry(srvx, "master_thread() end!");
    pthread_mutex_destroy(&srvx->server_mutex);
    pthread_mutex_destroy(&srvx->srvlog_mutex);
    pthread_mutex_destroy(&srvx->accept_mutex);
    #ifdef _WIN32
    WSACleanup();
    #endif
    srvx->status = SRVX_STOPPED;
    return 0;
}


int
main(int argc, char *argv[])
{
    struct srvx_t srvx = { 0 };
    srvx.f_addr = "127.0.0.1";
    srvx.f_port = "8000";
    srvx.d_addr = "127.0.0.1";
    srvx.d_port = "8080";
    srvx.n_worker_max = 100;
    master_thread(&srvx);
    return 0;
}
