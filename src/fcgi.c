
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "fcgi.h"


struct fcgi_pmgr_t *
fcgi_pmgr_new(struct htdx_t *htdx, char *extname, char *sz_addr, char *sz_port, char *sz_cmdl)
{
    struct fcgi_pmgr_t *fcgi_pmgr = calloc(1, sizeof(struct fcgi_pmgr_t));

    fcgi_pmgr->n_conn_max = 64;

    strncpy(fcgi_pmgr->cgiextname, extname,  15);
    strncpy(fcgi_pmgr->fcgid_addr, sz_addr,  63);
    strncpy(fcgi_pmgr->fcgid_port, sz_port,  15);
    strncpy(fcgi_pmgr->fcgid_cmdl, sz_cmdl, 255);

    fcgi_pmgr->rsa.u.sin.sin_family      = AF_INET;
    fcgi_pmgr->rsa.u.sin.sin_addr.s_addr =  inet_addr(fcgi_pmgr->fcgid_addr);
    fcgi_pmgr->rsa.u.sin.sin_port        = htons(atoi(fcgi_pmgr->fcgid_port));

    fcgi_pmgr->htdx = htdx;
    return fcgi_pmgr;
}


void
fcgi_pmgr_del(struct fcgi_pmgr_t *fcgi_pmgr)
{
    if (!fcgi_pmgr)
    {
        return;
    }
    fcgi_pmgr->htdx->fcgi_pmgr = NULL;
    free(fcgi_pmgr);
}


int
chtd_set_fcgi(struct htdx_t *htdx, char *extname, char *sz_addr, char *sz_port, char *sz_cmdl)
{
    int port = atoi(sz_port);

    if (port > 65535 || port < 1)
    {
        sz_port = "9000";
    }

    if (strlen(sz_addr) == 0)
    {
        sz_addr = "0.0.0.0";
    }

    char *temp = str_replace("@addr@", sz_addr, sz_cmdl);
    char *cmdl = str_replace("@port@", sz_port, temp);
    fcgi_pmgr_del(htdx->fcgi_pmgr);
    htdx->fcgi_pmgr = fcgi_pmgr_new(htdx, extname, sz_addr, sz_port, cmdl);
    free(temp);
    free(cmdl);
    return 1;
}


struct fcgi_conn_t *
fcgi_conn_new(struct reqs_t *http_reqs)
{
    struct      htdx_t      *htdx = http_reqs->htdx;
    struct fcgi_pmgr_t *fcgi_pmgr = htdx->fcgi_pmgr;

    SOCKET fcgi_socket;
    fcgi_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fcgi_socket == 0)
    {
        chtd_cry(htdx, "ERROR: fcgi_conn_new() -> socket() failed!%d");
        return NULL;
    }

    if (connect(fcgi_socket, &fcgi_pmgr->rsa.u.sa, sizeof(fcgi_pmgr->rsa.u)) == -1)
    {
        chtd_cry(htdx, "ERROR: connect() to fastcgi server (%s:%s) failed!",
                 fcgi_pmgr->fcgid_addr, fcgi_pmgr->fcgid_port);
        #ifdef WIN32
        closesocket(fcgi_socket);
        #else
        close(fcgi_socket);
        #endif
        return NULL;
    }

    struct fcgi_conn_t *fcgi_conn = calloc(1, sizeof(struct fcgi_conn_t));
    if (!fcgi_conn)
    {
        return NULL;
    }
    fcgi_conn->sock.socket  = fcgi_socket;
    fcgi_conn->http_reqs    = http_reqs;
    fcgi_conn->fcgi_reqs    = NULL;
    fcgi_conn->fcgi_pmgr    = fcgi_pmgr;
    fcgi_conn->htdx         = htdx;
    fcgi_conn->recvbufx     = bufx_new(4096, 1024*1024*8);
    return fcgi_conn;
}


void
fcgi_conn_del(struct fcgi_conn_t *fcgi_conn)
{
    if (!fcgi_conn)
    {
        return;
    }
    fcgi_conn_close(fcgi_conn);
    bufx_del(fcgi_conn->recvbufx);
    free(fcgi_conn);
}


void
fcgi_conn_close(struct fcgi_conn_t *fcgi_conn)
{
    if (!fcgi_conn)
    {
        return;
    }
    if (fcgi_conn->sock.socket != 0)
    {
        shutdown(fcgi_conn->sock.socket, SHUT_WR);
        static char buff[1024];
        while (recv(fcgi_conn->sock.socket, buff, 1024, 0) > 0);
        #ifdef WIN32
        closesocket(fcgi_conn->sock.socket);
        #else
        close(fcgi_conn->sock.socket);
        #endif
        fcgi_conn->sock.socket = 0;
    }
}


/*
功能: 发送 Size 字节数据;
描述: 必须发送足够 Size 字节才返回 1, 否则返回 0
返回: 1 成功, 0 失败;
*/
int
fcgi_conn_send(struct fcgi_conn_t *fcgi_conn, void *data, int size)
{
    int done = 0;
    while (done < size)
    {
        int retn = send(fcgi_conn->sock.socket, data + done, size - done, 0);
        if (retn > 0)
        {
            done += retn;
        }
        else
        {
            DEBUG_TRACE("fcgi_conn_send() -> send() return %d!", retn);
            return 0;
        }
    }
    return done;
}


/*
功能: 接收 need 字节数据;
描述: 必须收到足够 need 字节才返回 1, 否则放回缓存, 返回 0
返回: 1 成功, 0 失败;
*/
int
fcgi_conn_recv(struct fcgi_conn_t *fcgi_conn, void *buff, int need)
{
    int done = bufx_get(fcgi_conn->recvbufx, buff, need);
    while (done < need)
    {
        int retn = recv(fcgi_conn->sock.socket, buff + done, need - done, 0);
        if (retn > 0)
        {
            done += retn;
        }
        else
        {
            /* */
            bufx_put(fcgi_conn->recvbufx, buff, done);
            chtd_cry(fcgi_conn->htdx, "fcgi_conn_recv() -> recv() return %d", retn);
            return 0;
        }
    }

    return done;
}


struct fcgi_reqs_t *
fcgi_reqs_new(struct reqs_t *http_reqs)
{
    struct fcgi_conn_t *fcgi_conn = fcgi_conn_new(http_reqs);
    if (!fcgi_conn)
    {
        return NULL;
    }

    struct fcgi_reqs_t *fcgi_reqs = calloc(1, sizeof(struct fcgi_reqs_t));
    if (!fcgi_reqs)
    {
        fcgi_conn_del(fcgi_conn);
        return NULL;
    }

    int req_Id = 1;
    fcgi_conn->fcgi_reqs   = fcgi_reqs;
    fcgi_reqs->fcgi_conn   = fcgi_conn;
    fcgi_reqs->http_reqs   = http_reqs;
    fcgi_reqs->http_conn   = http_reqs->conn;
    fcgi_reqs->fcgi_pmgr   = http_reqs->htdx->fcgi_pmgr;
    fcgi_reqs->htdx        = http_reqs->htdx;
    fcgi_reqs->requestId   = req_Id;
    FCGI_Header *re_header = &fcgi_reqs->re_header;
    re_header->version     = FCGI_VERSION_1;
    re_header->requestIdB1 = req_Id >> 8;
    re_header->requestIdB0 = req_Id & 0xff;
    return fcgi_reqs;
}


void
fcgi_reqs_del(struct fcgi_reqs_t *fcgi_reqs)
{
    if (!fcgi_reqs)
    {
        return;
    }
    fcgi_conn_del(fcgi_reqs->fcgi_conn);
    free(fcgi_reqs);
}


int
fcgi_send_header(struct fcgi_reqs_t *fcgi_reqs)
{
    return fcgi_conn_send(fcgi_reqs->fcgi_conn, &fcgi_reqs->re_header, sizeof(FCGI_Header));
}


int
fcgi_recv_header(struct fcgi_reqs_t *fcgi_reqs)
{
    FCGI_Header *rp_header = &fcgi_reqs->rp_header;

    if (fcgi_conn_recv(fcgi_reqs->fcgi_conn, rp_header, sizeof(FCGI_Header)))
    {
        fcgi_reqs->rp_requestId     = (rp_header->requestIdB1     << 8) + rp_header->requestIdB0;
        fcgi_reqs->rp_contentLength = (rp_header->contentLengthB1 << 8) + rp_header->contentLengthB0;
        return 1;
    }
    else
    {
        memset(rp_header, 0, sizeof(FCGI_Header));
        return 0;
    }
}


int
fcgi_send_padding(struct fcgi_reqs_t *fcgi_reqs, int padl)
{
    if (padl > 8)
    {
        chtd_cry(fcgi_reqs->htdx, "fcgi_send_padding() padl: %d is greater than 8!", padl);
    }
    static char buff[8] = "\0\0\0\0\0\0\0\0";
    int left = padl;
    int step;
    int retn;
    while (left > 0)
    {
        step = left > 8 ? 8 : left;
        retn = fcgi_conn_send(fcgi_reqs->fcgi_conn, buff, step);
        DEBUG_TRACE("fcgi_send_padding() -> fcgi_conn_send() = %d", retn);

        if (retn > 0)
        {
            left -= retn;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}


int
fcgi_recv_padding(struct fcgi_reqs_t *fcgi_reqs, int padl)
{
    if (padl > 8)
    {
        chtd_cry(fcgi_reqs->htdx, "fcgi_recv_padding() padl: %d is greater than 8!", padl);
    }
    static char buff[8];
    int left = padl;
    int step;
    int retn;
    while (left > 0)
    {
        step = left > 8 ? 8 : left;
        retn = fcgi_conn_recv(fcgi_reqs->fcgi_conn, buff, step);
        if (retn > 0)
        {
            left -= retn;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}


int
fcgi_send_begin_record(struct fcgi_reqs_t *fcgi_reqs)
{
    FCGI_BeginRequestBody body;
    FCGI_Header *re_header = &fcgi_reqs->re_header;
    body.roleB1 = 0;
    body.roleB0 = FCGI_RESPONDER;
    body.flags  = 0; /* FCGI_KEEP_CONN; */
    body.reserved[0] = 0;
    body.reserved[1] = 0;
    body.reserved[2] = 0;
    body.reserved[3] = 0;
    body.reserved[4] = 0;
    re_header->type            = FCGI_BEGIN_REQUEST;
    re_header->contentLengthB1 = 0;
    re_header->contentLengthB0 = sizeof(FCGI_BeginRequestBody);
    re_header->paddingLength   = 0;
    if (!fcgi_send_header(fcgi_reqs))
    {
        return 0;
    }
    if (!fcgi_conn_send(fcgi_reqs->fcgi_conn, &body, sizeof(FCGI_BeginRequestBody)))
    {
        return 0;
    }
    return 1;
}


int
fcgi_add_params(struct fcgi_reqs_t *fcgi_reqs, char *n, char *v)
{
    if (!n || !v)
    {
        return 0;
    }

    int nl = strlen(n);
    int vl = strlen(v);

    if (!nl || !vl)
    {
        return 0;
    }

    char *p;

    int   buffsize = fcgi_reqs->params_buffsize;
    int   datasize = fcgi_reqs->params_datasize;
    char *databody = fcgi_reqs->params_databody;

    int needsize = nl + vl + 8;
    int newbsize;

    if (buffsize - datasize < needsize)
    {
        newbsize = buffsize ? buffsize * 2 : 4096;

        while (newbsize - datasize < needsize)
        {
            newbsize *= 2;
        }

        if (!buffsize)
        {
            p = calloc(newbsize, sizeof(char));
        }
        else
        {
            p = realloc(databody, newbsize);
        }

        if (!p)
        {
            return 0;
        }

        databody = p;
        buffsize = newbsize;
    }

    p = databody + datasize;

    if (nl < 0x7f)
    {
        *p++ = nl;
        datasize++;
    }
    else
    {
        *p++ = nl >> 24 | 0x80;
        *p++ = nl >> 16;
        *p++ = nl >>  8;
        *p++ = nl;
        datasize += 4;
    }

    if (vl < 0x7f)
    {
        *p++ = vl;
        datasize++;
    }
    else
    {
        *p++ = vl >> 24 | 0x80;
        *p++ = vl >> 16;
        *p++ = vl >>  8;
        *p++ = vl;
        datasize += 4;
    }

    memcpy(databody + datasize, n, nl);
    datasize += nl;

    memcpy(databody + datasize, v, vl);
    datasize += vl;

    fcgi_reqs->params_buffsize = buffsize;
    fcgi_reqs->params_datasize = datasize;
    fcgi_reqs->params_databody = databody;
    return 1;
}


int
fcgi_send_params(struct fcgi_reqs_t *fcgi_reqs)
{
    FCGI_Header *re_header = &fcgi_reqs->re_header;
    int size = fcgi_reqs->params_datasize;
    re_header->type = FCGI_PARAMS;

    if (size)
    {
        char padl;
        padl = size & 0x07;
        padl = padl ? 8 - padl : 0;
        re_header->contentLengthB1 = size >> 8;
        re_header->contentLengthB0 = size & 0xff;
        re_header->paddingLength   = padl;

        if (!fcgi_send_header(fcgi_reqs))
        {
            return 0;
        }

        if (!fcgi_conn_send(fcgi_reqs->fcgi_conn, fcgi_reqs->params_databody, size + padl))
        {
            return 0;
        }
    }

    re_header->contentLengthB1 = 0;
    re_header->contentLengthB0 = 0;
    re_header->paddingLength   = 0;

    if (!fcgi_send_header(fcgi_reqs))
    {
        return 0;
    }

    return 1;
}


int
fcgi_send_stdin(struct fcgi_reqs_t *fcgi_reqs, char *data, int size)
{
    FCGI_Header *re_header = &fcgi_reqs->re_header;
    re_header->type = FCGI_STDIN;

    /* [ send stdin */
    if (data && size)
    {
        int left = size;
        int done = 0;
        int step;
        int padl;

        while (left > 0)
        {
            step = left > 0x8000 ? 0x8000 : left;
            padl = step & 0x07;
            padl = padl ? 8 - padl : 0;
            re_header->contentLengthB1 = step >> 8;
            re_header->contentLengthB0 = step & 0xff;
            re_header->paddingLength   = padl;

            if (!fcgi_send_header(fcgi_reqs))
            {
                chtd_cry(fcgi_reqs->htdx, "fcgi_send_header() failed!");
                return 0;
            }

            if (!fcgi_conn_send(fcgi_reqs->fcgi_conn, data + done, step))
            {
                chtd_cry(fcgi_reqs->htdx, "fcgi_conn_send() failed!");
                return 0;
            }

            if (!fcgi_send_padding(fcgi_reqs, padl))
            {
                chtd_cry(fcgi_reqs->htdx, "fcgi_send_padding() failed!");
                return 0;
            }

            left = left - step;
            done = done + step;
        }
    }

    /* ] */

    /* [ end stdin */
    re_header->contentLengthB1 = 0;
    re_header->contentLengthB0 = 0;
    re_header->paddingLength   = 0;

    if (!fcgi_send_header(fcgi_reqs))
    {
        chtd_cry(fcgi_reqs->htdx, "fcgi_send_header() failed!");
        return 0;
    }

    /* ] */

    return 1;
}


int
fcgi_recv_http_header(struct fcgi_conn_t *fcgi_conn, char *buff, int buffsize)
{
    if (!fcgi_conn || !buff || buffsize < 2)
    {
        return 0;
    }
    int buffleft = buffsize - 1;
    int recvsize = 0;
    while (buffleft)
    {
        int retn = recv(fcgi_conn->sock.socket, buff + recvsize, buffleft, 0);
        if (retn > 0)
        {
            recvsize += retn;
            buffleft -= retn;
            buff[recvsize] = '\0';
            char *endp = strstr(buff, "\r\n\r\n");
            if (endp)
            {
                endp += 4;
                int headsize = endp - buff;
                int extrsize = recvsize - headsize;
                if (extrsize) /* put back to bufx */
                {
                    bufx_put(fcgi_conn->recvbufx, endp, extrsize);
                }
                *endp = '\0';
                return headsize;
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
    bufx_put(fcgi_conn->recvbufx, buff, recvsize);
    return 0;
}


int
fcgi_trans_header(struct fcgi_reqs_t *fcgi_reqs, char *header_str)
{
    struct namevalue_t *nvs = NULL;
    parse_header(&nvs, header_str);

    if (!nvs)
    {
        return 0;
    }

    struct namevalue_t *curr, *last;

    curr = nvs;

    last = curr->prev;

    while (1)
    {
        if (striequ(curr->n, "Status"))
        {
            set_status_line(fcgi_reqs->http_reqs, curr->v);
        }
        else
        {
            set_http_header(fcgi_reqs->http_reqs, curr->n, curr->v);
        }

        if (curr == last)
        {
            break;
        }

        curr = curr->next;
    }

    namevalues_destroy(&nvs);
    return 1;
}


int
fcgi_trans_stdout(struct fcgi_reqs_t *fcgi_reqs)
{
    int contleft = fcgi_reqs->rp_contentLength;

    if (contleft == 0)
    {
        return 0;
    }

    /*
    [ read response HTTP header
    */
    struct reqs_t *http_reqs = fcgi_reqs->http_reqs;

    if (!http_reqs->rp_header_sent)
    {
        char header_str[4096];
        int retn = fcgi_recv_http_header(fcgi_reqs->fcgi_conn, header_str, 4096 - 1);

        if (retn > 0)
        {
            contleft -= retn;
            fcgi_trans_header(fcgi_reqs, header_str); /* trans headers */
        }
        else
        {
            chtd_cry(fcgi_reqs->htdx, "fcgi_recv_http_header() return %d", retn);
            reqs_throw_status(http_reqs, 500, "fcgi_recv_http_header() got an error!");
            /* "500 Internal Server Error" */
            return 0;
        }

        send_http_header(http_reqs);
    }

    /*
    ]
    */

    /*
    [ read stdout content
    */
    char buffdata[8192];
    int  buffsize = sizeof(buffdata);
    int  buffleft = buffsize;
    int  buffused = 0;
    int  needrecv;
    int  sizerecv;

    while (contleft > 0)
    {
        needrecv = buffleft > contleft ? contleft : buffleft;
        sizerecv = fcgi_conn_recv(fcgi_reqs->fcgi_conn, buffdata + buffused, needrecv);

        if (sizerecv > 0)
        {
            buffused += sizerecv;
            buffleft -= sizerecv;
            contleft -= sizerecv;

            if (buffleft == 0)
            {
                DEBUG_TRACE("buffused=%d", buffused);
                send_http_chunk(http_reqs, buffdata, buffused);
                buffused = 0;
                buffleft = buffsize;
            }
            else if (contleft == 0)
            {
                send_http_chunk(http_reqs, buffdata, buffused);
                break;
            }
        }
        else
        {
            chtd_cry(fcgi_reqs->htdx, "fcgi_conn_recv() return %d! when reading stdout content!", sizerecv);
            break;
        }
    }

    /*
    ]
    */

    fcgi_recv_padding(fcgi_reqs, fcgi_reqs->rp_header.paddingLength);
    return 1;
}


int
fcgi_trans_stderr(struct fcgi_reqs_t *fcgi_reqs)
{
    int contsize = fcgi_reqs->rp_contentLength;

    if (contsize == 0)
    {
        return 0;
    }

    if (fcgi_reqs->stderrbufx == NULL)
    {
        fcgi_reqs->stderrbufx  = bufx_new(1024, 1024*1024*2);
    }

    char buffdata[1024];
    int  buffsize = sizeof(buffdata);
    int  contleft = contsize;
    int  needrecv;
    int  sizerecv;

    while (contleft > 0)
    {
        needrecv = contleft > buffsize ? buffsize : contleft;
        sizerecv = fcgi_conn_recv(fcgi_reqs->fcgi_conn, buffdata, needrecv);

        if (sizerecv > 0)
        {
            contleft -= sizerecv;
            bufx_put(fcgi_reqs->stderrbufx, buffdata, sizerecv);
        }
        else
        {
            chtd_cry(fcgi_reqs->htdx, "fcgi_trans_stderr() -> fcgi_conn_recv() return %d!", sizerecv);
            break;
        }
    }

    fcgi_recv_padding(fcgi_reqs, fcgi_reqs->rp_header.paddingLength);
    return 1;
}


int
fcgi_reqs_done(struct fcgi_reqs_t *fcgi_reqs)
{
    FCGI_EndRequestBody body;
    fcgi_conn_recv(fcgi_reqs->fcgi_conn, &body, sizeof(FCGI_EndRequestBody));
    struct reqs_t *http_reqs = fcgi_reqs->http_reqs;

    if (!http_reqs->rp_header_sent)
    {
        set_http_status (http_reqs, 200); /* "200 OK" */
        set_http_header (http_reqs, "Transfer-Encoding", "chunked");
        send_http_header(http_reqs);
    }

    /* [ send stderr */
    int stderr_len = bufx_get_used(fcgi_reqs->stderrbufx);

    if (stderr_len)
    {
        send_http_chunk(http_reqs, "\r\n", 2);
        send_http_chunk(http_reqs, bufx_link(fcgi_reqs->stderrbufx), stderr_len);
    }

    /* ] */
    send_http_chunk_end(http_reqs);
    return 1;
}


int
fcgi_reqs_proc(struct reqs_t *http_reqs, struct vhost_t *vhost)
{
    struct conn_t *http_conn = http_reqs->conn;
    struct fcgi_reqs_t *fcgi_reqs = fcgi_reqs_new(http_reqs);

    if (!fcgi_reqs)
    {
        set_keep_alive(http_reqs, 0);
        reqs_throw_status(http_reqs, 504, "connect to fastcgi server failed!");
        return 1;
    }

    fcgi_add_params(fcgi_reqs, "SCRIPT_FILENAME",      http_reqs->real_path);
    fcgi_add_params(fcgi_reqs, "SCRIPT_NAME",          http_reqs->request_path);
    fcgi_add_params(fcgi_reqs, "REQUEST_URI",          http_reqs->uri);
    fcgi_add_params(fcgi_reqs, "QUERY_STRING",         http_reqs->query_string);
    fcgi_add_params(fcgi_reqs, "REQUEST_METHOD",       http_reqs->method_name);
    fcgi_add_params(fcgi_reqs, "DOCUMENT_URI",         http_reqs->request_path);
    fcgi_add_params(fcgi_reqs, "DOCUMENT_ROOT",        http_reqs->docs_root);
    fcgi_add_params(fcgi_reqs, "SERVER_PROTOCOL",      "HTTP/1.1");
    fcgi_add_params(fcgi_reqs, "GATEWAY_INTERFACE",    "CGI/1.1");
    fcgi_add_params(fcgi_reqs, "REMOTE_ADDR",          http_conn->client_addr);
    fcgi_add_params(fcgi_reqs, "REMOTE_PORT",          http_conn->client_port);
    fcgi_add_params(fcgi_reqs, "SERVER_ADDR",          http_conn->server_addr);
    fcgi_add_params(fcgi_reqs, "SERVER_PORT",          http_conn->server_port);
    fcgi_add_params(fcgi_reqs, "SERVER_NAME",          get_http_header(http_reqs, "Host"));
    fcgi_add_params(fcgi_reqs, "SERVER_SOFTWARE",      http_reqs->htdx->SERVER_SOFTWARE);
    fcgi_add_params(fcgi_reqs, "REDIRECT_STATUS",      "200");
    fcgi_add_params(fcgi_reqs, "CONTENT_TYPE",         get_http_header(http_reqs, "Content-Type"));
    fcgi_add_params(fcgi_reqs, "CONTENT_LENGTH",       get_http_header(http_reqs, "Content-Length"));
    fcgi_add_params(fcgi_reqs, "HTTP_HOST",            get_http_header(http_reqs, "Host"));
    fcgi_add_params(fcgi_reqs, "HTTP_USER_AGENT",      get_http_header(http_reqs, "User-Agent"));
    fcgi_add_params(fcgi_reqs, "HTTP_ACCEPT",          get_http_header(http_reqs, "Accept"));
    fcgi_add_params(fcgi_reqs, "HTTP_ACCEPT_ENCODING", get_http_header(http_reqs, "Accept-Encoding"));
    fcgi_add_params(fcgi_reqs, "HTTP_ACCEPT_LANGUAGE", get_http_header(http_reqs, "Accept-Language"));
    fcgi_add_params(fcgi_reqs, "HTTP_ACCEPT_CHARSET",  get_http_header(http_reqs, "Accept-Charset"));
    fcgi_add_params(fcgi_reqs, "HTTP_CONNECTION",      get_http_header(http_reqs, "Connection"));
    fcgi_add_params(fcgi_reqs, "HTTP_KEEP_ALIVE",      get_http_header(http_reqs, "Keep-Alive"));
    fcgi_add_params(fcgi_reqs, "HTTP_COOKIE",          get_http_header(http_reqs, "Cookie"));
    fcgi_add_params(fcgi_reqs, "HTTP_REFERER",         get_http_header(http_reqs, "Referer"));
    fcgi_add_params(fcgi_reqs, "HTTP_CONTENT_TYPE",    get_http_header(http_reqs, "Content-Type"));
    fcgi_add_params(fcgi_reqs, "HTTP_CONTENT_LENGTH",  get_http_header(http_reqs, "Content-Length"));

    if (!reqs_read_post(http_reqs))
    {
        fcgi_reqs_del(fcgi_reqs);
        return 1;
    }

    /*
    [ send fastcgi request
    */
    fcgi_send_begin_record(fcgi_reqs);
    fcgi_send_params      (fcgi_reqs);
    fcgi_send_stdin       (fcgi_reqs, http_reqs->post_data, http_reqs->post_size);
    free(fcgi_reqs->params_databody);
    fcgi_reqs->params_buffsize = 0;
    fcgi_reqs->params_datasize = 0;
    fcgi_reqs->params_databody = NULL;
    /*
    ]
    */

    set_http_header(http_reqs, "Transfer-Encoding", "chunked");

    /*
    [ recv fastcgi response
    */
    int loop = 1;

    while (loop)
    {
        if (!fcgi_recv_header(fcgi_reqs))
        {
            DEBUG_TRACE("fcgi_recv_header() retn 0!");
            break;
        }

        char type = fcgi_reqs->rp_header.type;

        switch (type)
        {
            /* STDOUT */
        case FCGI_STDOUT:
            if (!fcgi_trans_stdout(fcgi_reqs))
            {
                loop = 0;
            }

            break;

            /* STDERR */
        case FCGI_STDERR:
            if (!fcgi_trans_stderr(fcgi_reqs))
            {
                loop = 0;
            }

            break;

            /* END_REQUEST */
        case FCGI_END_REQUEST:
            if (!fcgi_reqs_done(fcgi_reqs))
            {
                chtd_cry(fcgi_reqs->htdx, "fcgi error: fcgi_reqs_done");
            }

            loop = 0;
            break;

        default:
            chtd_cry(fcgi_reqs->htdx, "fcgi error: fcgi header tpye error!");
            send_http_chunk_end(http_reqs);
            loop = 0;
            break;
        }
    }

    /*
    ]
    */

    fcgi_reqs_del(fcgi_reqs);
    return 1;
}
