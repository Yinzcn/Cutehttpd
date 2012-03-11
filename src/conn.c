
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "conn.h"


struct conn_t *
conn_new(struct wker_t *wker) {
    struct conn_t *conn = calloc(1, sizeof(struct conn_t));
    if (conn == NULL) {
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
    if (conn == NULL) {
        return;
    }
    sock_close(conn->sock);
    bufx_del(conn->recvbufx);
    free(conn->reqs_head);
    free(conn);
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
            chtd_cry(conn->htdx, "conn_recv() -> recv() return %d", retn);
            bufx_put(conn->recvbufx, buff, done);
            return 0;
        }
    }
    return done;
}


void
conn_set_send_timeout(struct conn_t *conn, int msec)
{
    sock_set_send_timeout(conn->sock, msec);
}


void
conn_set_recv_timeout(struct conn_t *conn, int msec)
{
    sock_set_recv_timeout(conn->sock, msec);
}


void
conn_parse_addr(struct conn_t *conn)
{
    struct sock_t *sock;
    if (conn == NULL) {
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
conn_recv_reqs_head(struct conn_t *conn)
{
    int buffsize = 4096;
    int buffleft = buffsize - 1;
    char *buff = NULL;
    int sizerecv = 0;
    if (conn->reqs_head) {
        free(conn->reqs_head);
    }
    buff = calloc(buffsize, sizeof(char));
    conn->reqs_head = buff;

    sizerecv = bufx_get(conn->recvbufx, buff, buffleft);
    buffleft -= sizerecv;

    while (1) {
        char *endp = strstr(buff, "\r\n\r\n");
        if (endp) {
            char *bufp = buff;
            while (*bufp == '\r' || *bufp == '\n') {
                bufp++;
            }
            if (bufp != buff) {
                strcpy(buff, bufp);
                sizerecv -= bufp - buff;
                buffleft += bufp - buff;
                continue;
            } else {
                int size_get;
                int size_ext;
                endp += 4; /*  = strlen("\r\n\r\n")  */
                size_get = endp - buff;
                size_ext = sizerecv - size_get;
                if (size_ext) { /* push to bufx */
                    bufx_put(conn->recvbufx, endp, size_ext);
                }
                *endp = '\0';
                return size_get;
            }
        } else
		if (buffleft == 0) {
            chtd_cry(conn->htdx, "conn_recv_reqs_head() -> sizerecv %d", sizerecv);
            break;
        } else {
            int retn = recv(conn->sock->socket, buff + sizerecv, buffleft, 0);
            if (retn > 0) {
                sizerecv += retn;
                buffleft -= retn;
                buff[sizerecv] = '\0';
            } else {
                break;
            }
        }
    }

    bufx_put(conn->recvbufx, buff, sizerecv);

    conn->reqs_head = NULL;
    return 0;
}
