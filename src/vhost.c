
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "vhost.h"
#include "test_ext.h"


int
vhost_proc(struct reqs_t *http_reqs, struct vhost_t *vhost)
{
    struct stat st;
    char *reqs_real_path = calloc(strlen(vhost->real_root) + strlen(http_reqs->request_path) + 1, sizeof(char));
    char *p = reqs_real_path;
    http_reqs->real_path = reqs_real_path;
    http_reqs->docs_root = strdup(vhost->real_root);
    strcat(reqs_real_path, vhost->real_root);
    while (*p) p++;
    url_decode(http_reqs->request_path, p);
    path_tidy(p);
    p = reqs_real_path;
    while (*p) p++;
    p--;
    if (*p == '/' && p != reqs_real_path) {
        *p = '\0';
    }

    /* [ test.ext */
    if (strncmp(http_reqs->request_path, "/test.ext", 9) == 0) {
        test_ext(http_reqs);
        return 1;
    }
    /* ] */

    if (stat(reqs_real_path, &st) == 0) {
        if (S_IFDIR & st.st_mode) {
        /* [ is dir */
            http_list_dir(http_reqs, reqs_real_path);
            return 1;
        /* ] */
        } else
        if (S_IFREG & st.st_mode) {
        /* [ is file */
            #ifdef CHTD_FCGI
            struct fcgi_pmgr_t *fcgi_pmgr;
            fcgi_pmgr = fcgi_pmgr_match(http_reqs->htdx, x_ext_name(reqs_real_path));
            if (fcgi_pmgr) {
                fcgi_reqs_proc(fcgi_pmgr, http_reqs);
                return 1;
            }
            #endif
            http_send_file(http_reqs, reqs_real_path);
            return 1;
        /* ] */
        }
    }

    reqs_throw_status(http_reqs, 404, "404 Not Found");
    return 1;
}


void
free_vhosts(struct htdx_t *htdx)
{
    struct vhost_t *curr, *next, *last;
    if (!htdx->vhosts) {
        return;
    }
    curr = htdx->vhosts;
    last = curr->prev;
    while (1) {
        next = curr->next;
        free(curr->host);
        free(curr->root);
        free(curr->conf);
        free(curr->real_root);
        free(curr);
        if (curr == last) {
            break;
        }
        curr = next;
    }
    htdx->vhosts = NULL;
}


void
vhosts_add(struct htdx_t *htdx, char *szHost, char *szRoot, char *szConf, char *real_root)
{
    struct vhost_t *vhost;
    vhost = calloc(1, sizeof(struct vhost_t));
    vhost->host = strdup(szHost);
    vhost->root = strdup(szRoot);
    vhost->conf = strdup(szConf);
    vhost->real_root = strdup(real_root);
    if (htdx->vhosts) {
        vhost->prev = htdx->vhosts->prev;
        vhost->next = htdx->vhosts;
        vhost->prev->next = vhost;
        vhost->next->prev = vhost;
    } else {
        vhost->prev  = vhost;
        vhost->next  = vhost;
        htdx->vhosts = vhost;
    }
}


int
vhosts_del(struct htdx_t *htdx, struct vhost_t *vhost)
{
    if (!vhost) {
        return 0;
    }
    free(vhost->host);
    free(vhost->root);
    free(vhost->conf);
    free(vhost->real_root);
    if (vhost == vhost->next) {
        /* only one */
        free(vhost);
        htdx->vhosts = NULL;
        return 1;
    } else {
        if (vhost == htdx->vhosts) {
            /* the first */
            htdx->vhosts = vhost->next;
        }
        vhost->prev->next = vhost->next;
        vhost->next->prev = vhost->prev;
        free(vhost);
        return 1;
    }
}


struct vhost_t *
chtd_get_vhost(struct htdx_t *htdx, char *szHost) {
    struct vhost_t *curr, *last;
    if (!htdx->vhosts) {
        return NULL;
    }
    curr = htdx->vhosts;
    last = curr->prev;
    while (1) {
        if (strcasecmp(curr->host, szHost) == 0) {
            return curr;
        }
        if (curr == last) {
            break;
        }
        curr = curr->next;
    }
    return NULL;
}


int
chtd_set_vhost(struct htdx_t *htdx, char *szHost, char *szRoot, char *szConf)
{
    struct vhost_t *vhost;
    char *real_root;
    if (!htdx || !szHost) {
        return 0;
    }
    if (!strlen(szHost)) {
        return 0;
    }
    real_root = calloc(FILENAME_MAX + 1, sizeof(char));
    x_realpath(szRoot, real_root);
    vhost = chtd_get_vhost(htdx, szHost);
    if (vhost) { /* already exists */
        if (strlen(szRoot) || strlen(szConf)) { /* to update */
            if (strlen(szRoot)) {
                free(vhost->root);
                vhost->root = strdup(szRoot);
                free(vhost->real_root);
                vhost->real_root = real_root;
            }
            if (strlen(szConf)) {
                free(vhost->conf);
                vhost->conf = strdup(szConf);
            }
            return 1;
        } else { /* to delete */
            vhosts_del(htdx, vhost);
            free(real_root);
            return 1;
        }
    } else { /* new */
        if (strlen(szRoot) && strlen(szConf)) {
            vhosts_add(htdx, szHost, szRoot, szConf, real_root);
            free(real_root);
            return 1;
        }
    }
    return 0;
}


struct vhost_t *
chtd_vhost_match(struct reqs_t *http_reqs) {
    struct vhost_t *curr, *last;
    char *host, *temp;
    if (!http_reqs->htdx->vhosts) {
        return NULL;
    }
    host = strdup(get_http_header(http_reqs, "host"));
    temp = strchr(host, ':');
    if (temp) { *temp = 0; };
    curr = http_reqs->htdx->vhosts;
    last = curr->prev;
    while (1) {
        if (strcasecmp(curr->host, host) == 0) {
            /* match host (xx.xx.xx.xx) */
            http_reqs->htdx->vhosts = curr;
            free(host);
            return curr;
        }
        if (curr == last) {
            break;
        }
        curr = curr->next;
    }
    free(host);
    return chtd_get_vhost(http_reqs->htdx, "*");
}
