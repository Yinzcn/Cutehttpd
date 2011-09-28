
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "conn.h"


struct conn_t *
conn_new(struct wker_t *wker)
{
    struct conn_t *conn = calloc(1, sizeof(struct conn_t));
    if (!conn)
    {
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
    if (!conn)
    {
        return;
    }
    conn_close(conn);
    bufx_del(conn->recvbufx);
    free(conn->sock);
    free(conn->reqs_strs);
    free(conn);
}


void
conn_close(struct conn_t *conn)
{
    struct sock_t *sock = conn->sock;
    if (!sock)
    {
      return;
    }
    if (sock->socket > 0)
    {
        shutdown(sock->socket, SHUT_WR);
        static char buff[1024];
        while (recv(sock->socket, buff, 1024, 0) > 0);
        #ifdef WIN32
        closesocket(sock->socket);
        #else
        close(sock->socket);
        #endif
        sock->socket = 0;
    }
}


int
conn_send(struct conn_t *conn, void *data, int size)
{
    int done = 0;
    int retn;
    while (done < size)
    {
        retn = send(conn->sock->socket, data + done, size - done, 0);
        if (retn > 0)
        {
            done += retn;
        }
        else
        {
            /* */
            return 0;
        }
    }
    conn->nSent += done;
    return done;
}



int
conn_recv(struct conn_t *conn, void *buff, int need)
{
    int done = bufx_get(conn->recvbufx, buff, need);
    int left = need - done;
    int retn;
    while (done < need)
    {
        retn = recv(conn->sock->socket, buff + done, (left > 8192 ? 8192 : left), 0);
        if (retn > 0)
        {
            done += retn;
            left -= retn;
        }
        else
        {
            /* */
            chtd_cry(conn->htdx, "conn_recv() -> recv() return %d", retn);
            bufx_put(conn->recvbufx, buff, done);
            return 0;
        }
    }
    return done;
}


void
conn_parse_addr(struct conn_t *conn)
{
    if (!conn)
    {
        return;
    }
    struct sock_t *sock = conn->sock;

    /* server_addr */
    strcpy (conn->server_addr,   inet_ntoa(sock->lsa.u.sin.sin_addr));
    sprintf(conn->server_port, "%d", ntohs(sock->lsa.u.sin.sin_port));

    /* client_addr */
    strcpy (conn->client_addr,   inet_ntoa(sock->rsa.u.sin.sin_addr));
    sprintf(conn->client_port, "%d", ntohs(sock->rsa.u.sin.sin_port));
}


int
conn_read_until(struct conn_t *conn, char *need, char *buff, int buffsize)
{
    if (!conn || !need || !buff || buffsize < 2)
    {
        return 0;
    }
    int buffleft = buffsize - 1;
    int sizerecv = 0;
    while (buffleft)
    {
        int retn = recv(conn->sock->socket, buff + sizerecv, buffleft, 0);
        if (retn > 0)
        {
            sizerecv += retn;
            buffleft -= retn;
            buff[sizerecv] = '\0';
            char *endp = strstr(buff, need);
            if (endp)
            {
                endp += strlen(need);
                int size_get = endp - buff;
                int size_ext = sizerecv - size_get;
                if (size_ext) /* put back to bufx */
                {
                    bufx_put(conn->recvbufx, endp, size_ext);
                }
                *endp = '\0';
                return size_get;
            }
            else
            {
                continue;
            }
        }
        else
        {
            break;
        }
    }
    bufx_put(conn->recvbufx, buff, sizerecv);
    return 0;
}


int
conn_recv_reqs_strs(struct conn_t *conn)
{
    if (conn->reqs_strs)
    {
        free(conn->reqs_strs);
    }
    int buffsize = 4096;
    conn->reqs_strs = calloc(buffsize, sizeof(char));
    int retn = conn_read_until(conn, "\r\n\r\n", conn->reqs_strs, buffsize);
    if (retn)
    {
        return retn;
    }
    free(conn->reqs_strs);
    conn->reqs_strs = NULL;
    return 0;
}
