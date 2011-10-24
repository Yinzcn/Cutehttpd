
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "conn.h"


struct conn_t *
conn_new(struct wker_t *wker) {
    struct conn_t *conn = calloc(1, sizeof(struct conn_t));
    if (!conn) {
        chtd_cry(NULL, "conn_new() -> calloc() failed!");
        return NULL;
    }
    conn->recvbufx = bufx_new(4096, 1024*1024);
    conn->htdx = wker->htdx;
    conn->wker = wker;
    wker->conn = conn;
    return conn;
}


void
conn_del(struct conn_t *conn)
{
    if (!conn) {
        return;
    }
    conn_close(conn);
    bufx_del(conn->recvbufx);
    free(conn->sock);
    free(conn->reqs_head);
    free(conn);
}


void
conn_close(struct conn_t *conn)
{
    struct sock_t *sock = conn->sock;
    if (!sock) {
        return;
    }
    if (sock->socket > 0) {
#ifdef WIN32
        static char buff[1024];
        unsigned long u = 1;
        struct linger l;
        l.l_onoff  = 1;
        l.l_linger = 1;
        setsockopt(sock->socket, SOL_SOCKET, SO_LINGER, (void *)&l, sizeof(l));
        shutdown(sock->socket, SHUT_WR);
        ioctlsocket(sock->socket, FIONBIO, &u);
        while (recv(sock->socket, buff, 1024, 0) > 0);
        closesocket(sock->socket);
        sock->socket = 0;
#else
        static char buff[1024];
        struct linger l;
        l.l_onoff  = 1;
        l.l_linger = 1;
        setsockopt(sock->socket, SOL_SOCKET, SO_LINGER, (void *)&l, sizeof(l));
        shutdown(sock->socket, SHUT_WR);
        while (recv(sock->socket, buff, 1024, 0) > 0);
        close(sock->socket);
        sock->socket = 0;
#endif
    }
}


int
conn_send(struct conn_t *conn, char *data, int size)
{
    int done = 0;
    int retn;
    while (done < size) {
        retn = send(conn->sock->socket, data + done, size - done, 0);
        if (retn > 0) {
            done += retn;
        } else {
            /* */
            return 0;
        }
    }
    conn->nSent += done;
    return done;
}



int
conn_recv(struct conn_t *conn, char *buff, int need)
{
    int done = bufx_get(conn->recvbufx, buff, need);
    int left = need - done;
    int retn;
    while (done < need) {
        retn = recv(conn->sock->socket, buff + done, (left > 8192 ? 8192 : left), 0);
        if (retn > 0) {
            done += retn;
            left -= retn;
        } else {
            /* */
            chtd_cry(conn->htdx, "conn_recv() -> recv() return %d", retn);
            bufx_put(conn->recvbufx, buff, done);
            return 0;
        }
    }
    return done;
}


void
conn_set_recv_timeout(struct conn_t *conn, int msec)
{
#ifdef WIN32
    int tv = msec;
#else
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = msec * 1000;
#endif
    setsockopt(conn->sock->socket, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(tv));
}


void
conn_set_send_timeout(struct conn_t *conn, int msec)
{
#ifdef WIN32
    int tv = msec;
#else
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = msec * 1000;
#endif
    setsockopt(conn->sock->socket, SOL_SOCKET, SO_SNDTIMEO, (void *)&tv, sizeof(tv));
}


void
conn_parse_addr(struct conn_t *conn)
{
    struct sock_t *sock;
    if (!conn) {
        return;
    }
    sock = conn->sock;

    /* server_addr */
    strcpy (conn->server_addr,  inet_ntoa (sock->lsa.u.sin.sin_addr));
    sprintf(conn->server_port, "%d", ntohs(sock->lsa.u.sin.sin_port));

    /* client_addr */
    strcpy (conn->client_addr,  inet_ntoa (sock->rsa.u.sin.sin_addr));
    sprintf(conn->client_port, "%d", ntohs(sock->rsa.u.sin.sin_port));
}


int
conn_read_until(struct conn_t *conn, char *endw, char *buff, int buffsize)
{
    int buffleft;
    int sizerecv;
    if (!conn || !endw || !buff || buffsize < 2) {
        return 0;
    }
    buffleft = buffsize - 1;
    sizerecv = bufx_get(conn->recvbufx, buff, buffleft);
    chtd_cry(conn->htdx, "conn_read_until() -> bufx_get %d", sizerecv);
    buffleft -= sizerecv;
    while (buffleft) {
        int retn;
        char *endp = strstr(buff, endw);
        if (endp) {
            int size_get;
            int size_ext;
            endp += strlen(endw);
            size_get = endp - buff;
            size_ext = sizerecv - size_get;
            if (size_ext) { /* put back to bufx */
                chtd_cry(conn->htdx, "conn_read_until() -> bufx_put %d", size_ext);
                bufx_put(conn->recvbufx, endp, size_ext);
            }
            *endp = '\0';
            return size_get;
        }
        retn = recv(conn->sock->socket, buff + sizerecv, buffleft, 0);
        if (retn > 0) {
            sizerecv += retn;
            buffleft -= retn;
            buff[sizerecv] = '\0';
        } else {
            break;
        }
    }
    bufx_put(conn->recvbufx, buff, sizerecv);
    return 0;
}


int
conn_recv_reqs_head(struct conn_t *conn)
{
    int buffsize = 4096;
    int retn;
    if (conn->reqs_head) {
        free(conn->reqs_head);
    }
    conn->reqs_head = calloc(buffsize, sizeof(char));
    retn = conn_read_until(conn, "\r\n\r\n", conn->reqs_head, buffsize);
    if (retn) {
        return retn;
    }
    free(conn->reqs_head);
    conn->reqs_head = NULL;
    return 0;
}
