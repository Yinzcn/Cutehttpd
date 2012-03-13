
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "wker.h"


int
init_wkers(struct htdx_t *htdx)
{
    int n = htdx->max_workers;
    int i;
    struct wker_t *wkers = calloc(n, sizeof(struct wker_t));
    struct wker_t *wker;
    for (i = 0; i < n; i++) {
        wker = &wkers[i];
        wker->w_id      = i;
        wker->conn      = NULL;
        wker->htdx      = htdx;
        if (wker_create_thread(wker) == 0) {
            wker->status = WK_IDEL;
        } else {
            chtd_cry(htdx, "wker_create_thread() failed!");
            wker->status = WK_DEAD;
        }
        if (htdx->wkers) {
            wker->prev = htdx->wkers->prev;
            wker->next = htdx->wkers;
            wker->prev->next = wker;
            wker->next->prev = wker;
        } else {
            wker->prev  = wker;
            wker->next  = wker;
            htdx->wkers = wker;
        }
    }
    return 0;
}


int
free_wkers(struct htdx_t *htdx)
{
    if (htdx->wkers) {
        struct wker_t *curr, *last;
        if (htdx->n_worker_thread) {
            return -1;
        }
        curr = htdx->wkers;
        last = curr->prev;
        while (1) {
            conn_del(curr->conn);
            if (curr == last) {
                break;
            }
            curr = curr->next;
        }
        free(htdx->wkers);
        htdx->wkers = NULL;
    }
    return 0;
}


int
wker_create_thread(struct wker_t *wker)
{
    int maxtry = 10;
    pthread_t t_id;
    while (maxtry--) {
        if (pthread_create(&t_id, NULL, (void *)worker_thread, wker) == 0) {
            wker->t_id = t_id;
            return 0;
        }
        x_msleep(100);
    }
    chtd_cry(wker->htdx, "wker_create_thread() -> pthread_create() failed!");
    return -1;
}


int
wker_get_id(struct wker_t *wker)
{
    return wker->w_id;
}


void
wker_stat(struct htdx_t *htdx)
{
    int nConn = 0;
    int nReqs = 0;
    struct wker_t *curr, *last;
    curr = htdx->wkers;
    last = curr->prev;
    while (1) {
        nConn += curr->nConn;
        nReqs += curr->nReqs;
        if (curr == last) {
            break;
        }
        curr = curr->next;
    }
    htdx->nConn = nConn;
    htdx->nReqs = nReqs;
}
