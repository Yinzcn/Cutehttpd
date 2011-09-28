
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


int
get_idel_wker(struct htdx_t *htdx, struct wker_t **wker)
{
    if (!htdx || !htdx->wkers)
    {
        return 0;
    }

    pthread_mutex_lock(&htdx->mx_wk);
    while (htdx->nWaitWkers == 0
        && htdx->nIdelWkers == 0)
    {
        pthread_cond_wait(&htdx->cv_wk_idel, &htdx->mx_wk);
        if (htdx->status != CHTD_RUNNING)
        {
            pthread_mutex_unlock(&htdx->mx_wk);
            return 0;
        }
    }

    struct wker_t *curr, *last;
    curr = htdx->wkers;
    last = curr->prev;
    if (htdx->nWaitWkers)
    {
        while (1)
        {
            if (curr->status == WK_WAIT)
            {
                curr->status = WK_HUNG;
                htdx->nWaitWkers--;
                pthread_mutex_unlock(&htdx->mx_wk);
                *wker = curr;
                return 1;
            }
            if (curr == last)
            {
                break;
            }
            curr = curr->next;
        }
    }
    curr = htdx->wkers;
    while (1)
    {
        if (curr->status == WK_IDEL)
        {
            curr->status = WK_HUNG;
            htdx->nIdelWkers--;
            pthread_mutex_unlock(&htdx->mx_wk);
            *wker = curr;
            return 1;
        }
        if (curr == last)
        {
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&htdx->mx_wk);
    return 0;
}


int
put_idel_wker(struct htdx_t *htdx, struct wker_t **wker)
{
    pthread_mutex_lock(&htdx->mx_wk);
    if (htdx->nWaitWkers < 10)
    {
        (*wker)->status = WK_WAIT;
        htdx->nWaitWkers++;
    }
    else
    {
        if (htdx->nIdelWkers == 0) {
            pthread_cond_signal(&htdx->cv_wk_idel);
        }
        (*wker)->status = WK_IDEL;
        htdx->nIdelWkers++;
    }
    pthread_mutex_unlock(&htdx->mx_wk);
    return 1;
}


int
wker_create_thread(struct wker_t *wker)
{
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
