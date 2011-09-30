
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "wker.h"


int
init_wkers(struct htdx_t *htdx)
{
    int n = htdx->max_workers;
    int i;
    struct wker_t *wkers = calloc(n, sizeof(struct wker_t));
    struct wker_t *wker;
    htdx->nIdelWkers = htdx->max_workers;
    for (i = 0; i < n; i++)
    {
        wker = &wkers[i];
        wker->w_id      = i;
        wker->conn      = NULL;
        wker->htdx      = htdx;
        pthread_mutex_init(&wker->mx_wker, NULL);
        pthread_cond_init (&wker->cv_wake, NULL);
        if (wker_create_thread(wker) == 0) {
            wker->status = WK_IDEL;
        } else {
            chtd_cry(htdx, "wker_create_thread() failed!");
            wker->status = WK_DEAD;
        }
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
    return 0;
}


int
free_wkers(struct htdx_t *htdx)
{
    if (htdx->wkers)
    {
        if (htdx->nIdelWkers != htdx->max_workers)
        {
            return -1;
        }
        struct wker_t *curr, *last;
        curr = htdx->wkers;
        last = curr->prev;
        while (1)
        {
            conn_del(curr->conn);
            pthread_mutex_destroy(&curr->mx_wker);
            pthread_cond_destroy (&curr->cv_wake);
            if (curr == last)
            {
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
get_idel_wker(struct htdx_t *htdx, struct wker_t **wker)
{
    if (!htdx || !htdx->wkers)
    {
        chtd_cry(htdx, "get_idel_wker() -> (!htdx || !htdx->wkers)");
        return 0;
    }

    pthread_mutex_lock(&htdx->mx_wk);
    while (htdx->nIdelWkers == 0)
    {
        htdx->cv_wk_get_wait = 1;
        pthread_cond_wait(&htdx->cv_wk_idel, &htdx->mx_wk);
        htdx->cv_wk_get_wait = 0;
        if (htdx->status != CHTD_RUNNING)
        {
            chtd_cry(htdx, "get_idel_wker() htdx->status != CHTD_RUNNING");
            pthread_mutex_unlock(&htdx->mx_wk);
            return 0;
        }
    }

    struct wker_t *curr, *last;
    curr = htdx->wkers;
    last = curr->prev;
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
            chtd_cry(htdx, "get_idel_wker() htdx->nIdelWkers = %d, but failed!", htdx->nIdelWkers);
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
    (*wker)->status = WK_IDEL;
    htdx->nIdelWkers++;
    if (htdx->cv_wk_get_wait)
    {
        pthread_cond_signal(&htdx->cv_wk_idel);
    }
    pthread_mutex_unlock(&htdx->mx_wk);
    return 1;
}


int
wker_create_thread(struct wker_t *wker)
{
    pthread_t t_id;
    int maxtry = 10;
    while (maxtry--)
    {
        if (pthread_create(&t_id, NULL, (void *)worker_thread, wker) == 0)
        {
            wker->t_id = t_id;
            return 0;
        }
        sleep(100);
    }
    chtd_cry(wker->htdx, "wker_create_thread() -> pthread_create() failed!");
    return -1;
}


int
wker_wakeup(struct wker_t *wker)
{
    pthread_mutex_lock  (&wker->mx_wker);
    wker->status = WK_WKUP;
    pthread_cond_signal (&wker->cv_wake);
    pthread_mutex_unlock(&wker->mx_wker);
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
