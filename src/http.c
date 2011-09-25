
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "http.h"


int
set_status_line(struct reqs_t *http_reqs, char *status_line)
{
    if (!http_reqs || !status_line)
    {
        chtd_cry(NULL, "call set_status_line() with NULL http_reqs OR NULL status_line!");
        return 0;
    }
    char buff[256] = "\0";
    if (strlen(status_line) > 255)
    {
        return 0;
    }
    strcat(buff, "HTTP/1.1 ");
    strcat(buff, status_line);
    if (http_reqs->rp_status_line)
    {
        free(http_reqs->rp_status_line);
    }
    http_reqs->rp_status_line = strdup(buff);
    return 1;
}


int
set_http_status(struct reqs_t *http_reqs, int status_code)
{
    return set_status_line(http_reqs, http_status_lines_get(status_code));
}


int
set_http_header(struct reqs_t *http_reqs, char *n, char *v)
{
    if (!http_reqs || !n || !v)
    {
        return 0;
    }
    int nl = strlen(n);
    int vl = strlen(v);
    if (nl)
    {
        if (striequ(n, "Set-Cookie"))
        {
            if (vl)
            {
                namevalues_add(&http_reqs->rp_headers, n, nl, v, vl);
            }
        }
        else
        {
            struct namevalue_t *nv = namevalues_get(&http_reqs->rp_headers, n);
            if (nv)   /* already exists, delete it */
            {
                namevalues_del(&http_reqs->rp_headers, nv);
            }
            if (vl)   /* add */
            {
                namevalues_add(&http_reqs->rp_headers, n, nl, v, vl);
            }
        }
    }
    return 1;
}


int
set_http_header_x(struct reqs_t *http_reqs, char *n, char *f, ...)
{
    if (!http_reqs || !n || !f)
    {
        return 0;
    }
    if (!strlen(n))
    {
        return 0;
    }
    if (strlen(f))
    {
        char b[4096];
        va_list a;
        va_start(a, f);
        vsprintf(b, f, a);
        va_end(a);
        set_http_header(http_reqs, n, b);
    }
    return 1;
}


int
set_keep_alive(struct reqs_t *reqs, int should_keep_alive)
{
    if (!reqs)
    {
        chtd_cry(NULL, "set_keep_alive() with NULL reqs!");
        return 0;
    }
    if (reqs->htdx->keep_alive_timeout > 0
            && striequ(get_http_header(reqs, "Connection"), "keep-alive")
            && reqs->http_version == HTTP_VERSION_1_1
            && should_keep_alive)
    {
        set_http_header   (reqs, "Connection", "keep-alive");
        set_http_header_x (reqs, "Keep-Alive", "timeout=%d", reqs->htdx->keep_alive_timeout);
        reqs->conn->keep_alive = 1;
        return 1;
    }
    else
    {
        set_http_header   (reqs, "Connection", "Close");
        set_http_header   (reqs, "Keep-Alive", "");
        reqs->conn->keep_alive = 0;
        return 1;
    }
}


char *
get_http_header(struct reqs_t *http_reqs, char *name)
{
    struct namevalue_t *nv;
    nv = namevalues_get(&http_reqs->re_headers, name);
    if (nv)
    {
        return nv->v;
    }
    return "";
}


int
send_http_header(struct reqs_t *http_reqs)
{
    struct bufx_t *bufx = bufx_new(4096, 1024*1024);
    if (!http_reqs->rp_status_line)
    {
        set_http_status(http_reqs, 200); /* "200 OK" */
    }
    bufx_put_str(bufx, http_reqs->rp_status_line);
    bufx_put_str(bufx, "\r\n");
    if (http_reqs->rp_headers)
    {
        struct namevalue_t *curr, *last;
        curr = http_reqs->rp_headers;
        last = curr->prev;
        while (1)
        {
            bufx_put_str(bufx, curr->n);
            bufx_put_str(bufx, ": ");
            bufx_put_str(bufx, curr->v);
            bufx_put_str(bufx, "\r\n");
            if (curr == last)
            {
                break;
            }
            curr = curr->next;
        }
    }
    bufx_put_str(bufx, "\r\n");
    bufx_get_each(bufx, reqs_conn_send, http_reqs);
    http_reqs->rp_header_sent = 1;
    bufx_del(bufx);
    return 1;
}


int
send_http_chunk(struct reqs_t *reqs, void *data, int size)
{
    struct conn_t *conn = reqs->conn;
    char head[16];
    itoa(size, head, 16);
    strcat(head, "\r\n");
    int n = 0;
    n += conn_send(conn, head, strlen(head));
    n += conn_send(conn, data, size);
    n += conn_send(conn, "\r\n", 2);
    return n;
}


int
send_http_chunk_end(struct reqs_t *reqs)
{
    return reqs_conn_send(reqs, "0\r\n\r\n", 5);
}
