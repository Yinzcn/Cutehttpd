
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "wker.h"


void
init_wkers(struct htdx_t *htdx)
{
    int n = htdx->max_workers;
    int i;
    struct wker_t *wkers = calloc(n, sizeof(struct wker_t));
    struct wker_t *wker;
    for (i = 0; i < n; i++)
    {
        wker = &wkers[i];
        wker->w_id      = i;
        wker->conn      = NULL;
        wker->htdx      = htdx;
        wker->status    = WK_IDEL;
        pthread_mutex_init(&wker->mx_wake, NULL);
        pthread_cond_init (&wker->cv_wake, NULL);
        if (htdx->wkers)
        {
            wker->prev = htdx->wkers->prev;
            wker->next = htdx->wkers;
            wker->prev->next = wker;
            wker->next->prev = wker;
        }
        else
        {
            wker->prev  = wker;
            wker->next  = wker;
            htdx->wkers = wker;
        }
    }
    htdx->nIdelWkers = htdx->max_workers;
}


void
free_wkers(struct htdx_t *htdx)
{
    if (htdx->wkers)
    {
        struct wker_t *curr, *last;
        curr = htdx->wkers;
        last = curr->prev;
        while (1)
        {
            conn_del(curr->conn);
            free(curr);
            if (curr == last)
            {
                break;
            }
            curr = curr->next;
        }
        htdx->wkers = NULL;
    }
}


struct wker_t *
get_idel_wker(struct htdx_t *htdx)
{
    if (!htdx || !htdx->wkers)
    {
        return NULL;
    }

    pthread_mutex_lock(&htdx->mx_wk);
    while (htdx->nIdelWkers == 0) {
        pthread_cond_wait(&htdx->cv_wk_idel, &htdx->mx_wk);
    }

    struct wker_t *curr, *last;
    curr = htdx->wkers;
    last = curr->prev;
    while (1)
    {
        if (curr->status == WK_WAIT)
        {
            curr->status = WK_HUNG;
            htdx->nIdelWkers--;
            pthread_mutex_unlock(&htdx->mx_wk);
            return curr;
        }
        if (curr == last)
        {
            break;
        }
        curr = curr->next;
    }
    curr = htdx->wkers;
    while (1)
    {
        if (curr->status == WK_IDEL)
        {
            curr->status = WK_HUNG;
            htdx->nIdelWkers--;
            pthread_mutex_unlock(&htdx->mx_wk);
            return curr;
        }
        if (curr == last)
        {
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&htdx->mx_wk);
    return NULL;
}


void
put_idel_wker(struct wker_t *wker)
{
    struct htdx_t *htdx = wker->htdx;
    pthread_mutex_lock(&htdx->mx_wk);
    ;
    /*htdx->wkers = wker;*/
    if (htdx->nIdelWkers == 0) {
        htdx->nIdelWkers = 1;
        pthread_cond_signal(&htdx->cv_wk_idel);
    }
    else
    {
        if (wker->status != WK_WAIT) {
            htdx->nIdelWkers++;
        }
    }
    wker->status = WK_IDEL;
    pthread_mutex_unlock(&htdx->mx_wk);
}


void
put_wait_wker(struct wker_t *wker)
{
    struct htdx_t *htdx = wker->htdx;
    pthread_mutex_lock(&htdx->mx_wk);
    /*htdx->wkers = wker;*/
    if (htdx->nIdelWkers == 0) {
        htdx->nIdelWkers = 1;
        pthread_cond_signal(&htdx->cv_wk_idel);
    } else {
        htdx->nIdelWkers++;
    }
    wker->status = WK_WAIT;
    pthread_mutex_unlock(&htdx->mx_wk);
}


int
wker_create_thread(struct wker_t *wker)
{
    DEBUG_TRACE("wker_create_thread(%d)", wker->w_id);
    pthread_t t_id;
    if (pthread_create(&t_id, NULL, (void *)worker_thread, wker) == 0)
    {
        wker->t_id = t_id;
        return 1;
    }
    return 0;
}


int
wker_wake(struct wker_t *wker)
{
    wker->status = WK_HUNG;
    chtd_cry(wker->htdx, "wker->birthtime = %d!", wker->birthtime);
    if (wker->birthtime == 0)
    {
        wker_create_thread(wker);
    } else {
        pthread_cond_signal(&wker->cv_wake);
    }
    return 1;
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
    while (1)
    {
        nConn += curr->nConn;
        nReqs += curr->nReqs;
        if (curr == last)
        {
            break;
        }
        curr = curr->next;
    }
    htdx->nConn = nConn;
    htdx->nReqs = nReqs;
}
