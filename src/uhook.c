
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "uhook.h"
#ifdef HAVE_PCRE
#include "pcre.h"
#endif

struct uhook_t *
uhooks_add(struct htdx_t *htdx, char *host, char *xuri, void *func, void *pPcre) {
    struct uhook_t *uhook;
    uhook = calloc(1, sizeof(struct uhook_t));
    uhook->host = strdup(host);
    uhook->xuri = strdup(xuri);
    uhook->func = func;
    uhook->pPcre = pPcre;
    if (htdx->uhooks) {
        uhook->prev = htdx->uhooks->prev;
        uhook->next = htdx->uhooks;
        uhook->prev->next = uhook;
        uhook->next->prev = uhook;
        return htdx->uhooks;
    } else {
        uhook->prev = uhook;
        uhook->next = uhook;
        return uhook;
    }
}


struct uhook_t *
uhooks_del(struct htdx_t *htdx, struct uhook_t *uhooks, struct uhook_t *uhook) {
    void *retn = uhooks;
    if (uhooks && uhook) {
        free(uhook->host);
        free(uhook->xuri);
        free(uhook->pPcre);
        if (uhook->next == uhook) {
            /* [ only one */
            retn = NULL;
            /* ] */
        } else {
            if (uhook == uhooks) {
                /* [ the first */
                retn = uhook->next;
                /* ] */
            }
            uhook->prev->next = uhook->next;
            uhook->next->prev = uhook->prev;
        }
        free(uhook);
    }
    return retn;
}


struct uhook_t *
uhooks_get(struct uhook_t *uhooks, char *host, char *xuri) {
    struct uhook_t *curr, *last;
    if (!uhooks) {
        return NULL;
    }
    curr = uhooks;
    last = curr->prev;
    while (1) {
        if (striequ(curr->host, host) && striequ(curr->xuri, xuri)) {
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
free_uhooks(struct htdx_t *htdx)
{
    if (htdx->uhooks) {
        struct uhook_t *curr, *last;
        curr = htdx->uhooks;
        last = curr->prev;
        while (1) {
            free(curr->host);
            free(curr->xuri);
            free(curr->pPcre);
            free(curr);
            if (curr == last) {
                break;
            }
            curr = curr->next;
        }
    }
    return 0;
}


struct uhook_t *
chtd_get_uhook(struct htdx_t *htdx, char *host, char *xuri) {
    return uhooks_get(htdx->uhooks, host, xuri);
}


int
chtd_set_uhook(struct htdx_t *htdx, char *host, char *xuri, void *func)
{
    struct uhook_t *uhook;
    if (func) {
        void *pPcre = NULL;
#ifdef HAVE_PCRE
        /* [ pcre */
        int nOffset = -1;
        const char *pErrMsg = NULL;
        pPcre = pcre_compile(xuri, 0, &pErrMsg, &nOffset, NULL);
        if (!pPcre) {
            chtd_cry(htdx, "set_uhook: pcre_compile() retn: ErrMsg=%s, Offset=%d xuri[%s]", pErrMsg, nOffset, xuri);
            return 0;
        }
        /* ] */
#endif
        uhook = uhooks_get(htdx->uhooks, host, xuri);
        if (uhook) { /* already exists */
            /* [ update it */
            uhook->func = func;
            return 1;
            /* ] */
        } else {
            /* [ new */
            htdx->uhooks = uhooks_add(htdx, host, xuri, func, pPcre);
            return 1;
            /* ] */
        }
    } else {
        /* [ to del */
        uhook = uhooks_get(htdx->uhooks, host, xuri);
        if (uhook) {
            htdx->uhooks = uhooks_del(htdx, htdx->uhooks, uhook);
            return 1;
        } else {
            return 0;
        }
        /* ] */
    }
}


struct uhook_t *
chtd_uhook_match(struct reqs_t *reqs) {
    struct htdx_t *htdx = reqs->htdx;
    if (htdx->uhooks) {
        struct uhook_t *curr, *last;
        char *host, *temp;
        host = strdup(get_http_header(reqs, "Host"));
        temp = strchr(host, ':');
        if (temp) { *temp = 0; };
        curr = htdx->uhooks;
        last = curr->prev;
        while (1) {
            /* [ */
            if (strcasecmp(curr->host, host) == 0 || strcasecmp(curr->host, "*") == 0) {
#ifdef HAVE_PCRE
                if (pcre_exec(curr->pPcre, NULL, reqs->uri, strlen(reqs->uri), 0, 0, NULL, 0) == 0) {
                    free(host);
                    return curr;
                }
#else
                if (strcmp(curr->xuri, reqs->uri) == 0) {
                    free(host);
                    return curr;
                }
#endif
            }
            /* ] */
            if (curr == last) {
                break;
            }
            curr = curr->next;
        }
        free(host);
    }
    return NULL;
}
