
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "reqs.h"


static char *http_method_names[] = {
    "UNKNOWN",
    "GET",
    "POST",
    "HEAD",
    "PUT",
    "DELETE",
    "TRACE",
    "CONNECT",
    "OPTIONS"
};


static char *http_version_names[] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1"
};


struct reqs_t *
reqs_new(struct conn_t *conn)
{
    struct reqs_t *reqs;
    if (!conn) {
        chtd_cry(NULL, "called reqs_new() with NULL conn!");
    }
    reqs = calloc(1, sizeof(struct reqs_t));
    if (!reqs) {
        chtd_cry(conn->htdx, "alloc reqs failed!");
        return NULL;
    }
    reqs->htdx = conn->htdx;
    reqs->wker = conn->wker;
    reqs->conn = conn;
    set_http_header(reqs, "Server", reqs->htdx->SERVER_SOFTWARE);
    return reqs;
}


void
reqs_del(struct reqs_t *reqs)
{
    if (!reqs) {
        chtd_cry(NULL, "called reqs_del() with NULL reqs!");
        return;
    }
    namevalues_destroy(&reqs->re_headers);
    namevalues_destroy(&reqs->rp_headers);
    free(reqs->reqs_line);
    free(reqs->uri);
    free(reqs->request_path);
    free(reqs->query_string);
    free(reqs->real_path);
    free(reqs->docs_root);
    free(reqs->post_data);
    free(reqs->rp_status_line);
    free(reqs);
}


struct htdx_t *
reqs_get_htdx(struct reqs_t *reqs) {
    return reqs->htdx;
}


struct conn_t *
reqs_get_conn(struct reqs_t *reqs) {
    return reqs->conn;
}


struct wker_t *
reqs_get_wker(struct reqs_t *reqs) {
    return reqs->conn->wker;
}


int
reqs_conn_send(struct reqs_t *reqs, void *data, int size)
{
    return conn_send(reqs->conn, data, size);
}


void
reqs_throw_status(struct reqs_t *reqs, int status_code, char *msg)
{
    if (status_code > 199 && status_code != 204 && status_code != 304) {
        char buf[32];
        int  len = strlen(msg);
        if (!len) {
            msg = http_status_lines_get(status_code);
            len = strlen(msg);
        }
        sprintf(buf, "%d", len);
        set_http_status   (reqs, status_code);
        set_http_header   (reqs, "Content-Type",   "text/html");
        set_http_header   (reqs, "Content-Length", buf);
        send_http_header  (reqs);
        reqs_conn_send    (reqs, msg, len);
    } else {
        set_http_status   (reqs, status_code);
        set_http_header   (reqs, "Content-Type",   "");
        set_http_header   (reqs, "Content-Length", "");
        send_http_header  (reqs);
    }
}


int
reqs_read_post(struct reqs_t *reqs)
{
    char *post_data;
    int size_recv;
    int post_size = reqs->content_length;
    if (post_size == 0) {
        reqs->post_read_flag = 1;
        return 1;
    }
    if (reqs->post_read_flag) {
        return 1;
    }
    if (post_size > reqs->htdx->max_post_size || post_size < 0) {
        set_keep_alive(reqs, 0);
        reqs_throw_status(reqs, 400, "post data too large!");
        return 0;
    }
    post_data = calloc(post_size + 1, sizeof(char));
    size_recv = conn_recv(reqs->conn, post_data, post_size);
    if (size_recv != post_size) {
        free(post_data);
        set_keep_alive(reqs, 0);
        reqs_throw_status(reqs, 500, "read post data error!");
        chtd_cry(reqs->htdx, "recv post data error [%d]", size_recv);
        return 0;
    }
    reqs->post_size = post_size;
    reqs->post_data = post_data;
    reqs->post_read_flag = 1;
    return 1;
}


int
reqs_parse_post(struct reqs_t *reqs)
{
    char *content_type;
    char *charset = NULL;
    char *p;
    if (!reqs_read_post(reqs)) {
        return 0;
    }
    if (!reqs->post_size) {
        return 0;
    }
    content_type = strdup(get_http_header(reqs, "Content-Type"));
    p = content_type;
    while ((p = strchr(p, ';'))) {
        *p++ = '\0';
        while (*p == ' ') {
            p++;
        }
        if (strncmp(p, "charset=", 8) == 0) {
            charset = p + 8;
        }
    }
    charset = charset;
    if (strcasecmp(content_type, "application/x-www-form-urlencoded") != 0) {
        /* "a=Alpha&b=Beta" */
        char *n_a, *n_z;
        char *v_a, *v_z;
        int n_l = 0, v_l = 0;
        char *p = reqs->post_data;
        while (*p) {
            n_a = p;
            while (*p && *p != '=') {
                p++;
            }
            n_z = p;
            if (*p) {
                p++;
            } else {
                break;
            }
            v_a = p;
            while (*p && *p != '&') {
                p++;
            }
            v_z = p;
            if (*p) {
                p++;
            }
            n_l = n_z - n_a;
            v_l = v_z - v_a;
            if (n_l > 0 && v_l > 0) {
                namevalues_add(&reqs->post_vars, n_a, n_l, v_a, v_l);
            }
        }
    }
    free(content_type);
    return 1;
}


void
reqs_skip_post(struct reqs_t *reqs)
{
    char static buff[1024];
    int size = reqs->content_length;
    while (size) {
        size -= conn_recv(reqs->conn, buff, size > 1024 ? 1024 : size);
    }
    reqs->post_read_flag = 1;
}


int
reqs_parse(struct reqs_t *reqs)
{
    register char *a, *z, *p;
    char *reqs_head = reqs->conn->reqs_head;
    char *reqs_line;

    /*
    [ take reqs_line
    */
    a = reqs_head;
    while (*a == LF || *a == CR || *a == SP || *a == HT) {
        a++;
    }
    z = a;
    while (*z != LF && *z != CR && *z) {
        z++;
    }
    if (z == a) {
        return 0;
    }
    reqs_line = strndup(a, z - a);
    reqs->reqs_line = reqs_line;
    /*
    ]
    */

    /*
    [ method_name
    */
    reqs->method = HTTP_METHOD_UNKNOWN;

    a = reqs_line;
    z = a;
    while (*z != SP && *z) {
        z++;
    }

    switch (z - a) {
    case 0:
    case 1:
    case 2:
        break;

    case 3:
        if (str3equ(a, 'G', 'E', 'T')) {
            reqs->method = HTTP_METHOD_GET;
        } else
        if (str3equ(a, 'P', 'U', 'T')) {
            reqs->method = HTTP_METHOD_PUT;
        }
        break;

    case 4:
        if (str4equ(a, 'P', 'O', 'S', 'T')) {
            reqs->method = HTTP_METHOD_POST;
        } else
        if (str4equ(a, 'H', 'E', 'A', 'D')) {
            reqs->method = HTTP_METHOD_HEAD;
        }
        break;

    case 5:
        if (str5equ(a, 'T', 'R', 'A', 'C', 'E')) {
            reqs->method = HTTP_METHOD_TRACE;
        }
        break;

    case 6:
        if (str6equ(a, 'D', 'E', 'L', 'E', 'T', 'E')) {
            reqs->method = HTTP_METHOD_DELETE;
        }
        break;

    case 7:
        if (str7equ(a, 'O', 'P', 'T', 'I', 'O', 'N', 'S')) {
            reqs->method = HTTP_METHOD_OPTIONS;
        } else
        if (str7equ(a, 'C', 'O', 'N', 'N', 'E', 'C', 'T')) {
            reqs->method = HTTP_METHOD_CONNECT;
        }
        break;

    default:
        break;
    }
    reqs->method_name = http_method_names[reqs->method];
    if (reqs->method == HTTP_METHOD_UNKNOWN) {
        reqs_throw_status(reqs, 501, ""); /* "501 Method Not Implemented" */
        chtd_cry(reqs->htdx, "Method Not Implemented: [%s]", a);
        return 0;
    }
    /*
    ]
    */

    /*
    [ URI
    */
    #define is_valid_uri_char(c) ((c > 32) && (c != 127) && (c != 255))
    a = z;
    while (*a == SP) {
        a++;
    }
    z = a;
    while (*z != SP && *z) {
        z++;
    }
    if (a == z) {
        return 0;
    }
    reqs->uri = strndup(a, z - a);
    p = reqs->uri;
    while (*p) {
        if (!is_valid_uri_char(*p)) {
            return 0;
        }
        p++;
    }
    /*
    ]
    */

    /*
    [ http version
    */
    reqs->http_version = HTTP_VERSION_0_9;
    a = z;
    while (*a == SP) {
        a++;
    }
    z = a;
    while (*z) {
        z++;
    }
    if (z - a == 8) {
        if (str8equ(a, 'H', 'T', 'T', 'P', '/', '1', '.', '0')) {
            reqs->http_version = HTTP_VERSION_1_0;
        } else if (str8equ(a, 'H', 'T', 'T', 'P', '/', '1', '.', '1')) {
            reqs->http_version = HTTP_VERSION_1_1;
        }
    }
    reqs->http_version_name = http_version_names[reqs->http_version];
    /*
    ]
    */

    /*
    [ parse request_path
    */
    a = reqs->uri;
    z = a;
    while (*z && *z != '?') {
        z++;
    }
    reqs->request_path = strndup(a, z - a);
    path_tidy(reqs->request_path);
    /*
    ]
    */

    /*
    [ parse query_string
    */
    a = strchr(reqs->uri, '?');
    if (a) {
        reqs->query_string = strdup(a + 1);
    } else {
        reqs->query_string = calloc(1 , 1);
    }
    /*
    ]
    */

    /*
    [ http headers
    */
    parse_header(&reqs->re_headers, strchr(reqs_head, LF) + 1);
    /*
    ]
    */

    reqs->content_length = atoi(get_http_header(reqs, "Content-Length"));

    reqs_parse_post(reqs);

    return 1;
}


int
reqs_proc(struct conn_t *conn)
{
    struct vhost_t *vhost;
    struct uhook_t *uhook;
    struct reqs_t *reqs;
    reqs = reqs_new(conn);
    if (!reqs) {
        return 0;
    }

    if (!reqs_parse(reqs)) {
        conn->keep_alive = 0;
        reqs_throw_status(reqs, 400, ""); /* "400 Bad Request" */
        chtd_log(reqs->htdx, "reqs_parse() failed!");
        reqs->htdx->nBadReqs++;
        reqs_del(reqs);
        return 1;
    }

    reqs->wker->nReqs++;

    set_keep_alive(reqs, 1);

    /* [ match uhook */
    uhook = chtd_uhook_match(reqs);
    if (uhook) {
        if (uhook->func(reqs)) {
            if (reqs->rp_header_sent == 0) {
                reqs_throw_status(reqs, 400, ""); /* "400 Bad Request" */
            }
            if (reqs->post_read_flag == 0) {
                reqs_skip_post(reqs);
            }
            reqs_del(reqs);
            return 1;
        }
    }
    /* ] */

    /* [ match vhost */
    vhost = chtd_vhost_match(reqs);
    if (vhost) {
        if (vhost_proc(reqs, vhost)) {
            if (reqs->post_read_flag == 0) {
                reqs_skip_post(reqs);
            }
            reqs_del(reqs);
            return 1;
        }
    }
    /* ] */

    /* [ Bad Request */
    set_keep_alive(reqs, 0);
    reqs_throw_status(reqs, 400, ""); /* "400 Bad Request" */
    chtd_log(reqs->htdx, "Got a bad request.");
    reqs->htdx->nBadReqs++;
    /* ] */

    reqs_del(reqs);
    return 1;
}
