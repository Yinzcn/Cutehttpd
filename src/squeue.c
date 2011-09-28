
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "squeue.h"


int
init_squeue(struct htdx_t *htdx)
{
    int n = htdx->squeue_size;
    struct squeue_t *squeue = calloc(n, sizeof(struct squeue_t));
    if (!squeue)
    {
        return 0;
    }
    struct squeue_t *q;
    int i;
    for (i = 0; i < n; i++)
    {
        q = &squeue[i];
        if (htdx->squeue)
        {
            q->prev = htdx->squeue->prev;
            q->next = htdx->squeue;
            q->prev->next = q;
            q->next->prev = q;
        }
        else
        {
            q->prev  = q;
            q->next  = q;
            htdx->squeue = q;
        }
    }
    htdx->squeue_a = NULL;
    htdx->squeue_z = NULL;
    return 1;
}


void
free_squeue(struct htdx_t *htdx)
{
    if (htdx)
    {
        free(htdx->squeue);
        htdx->squeue = NULL;
    }
}


int
squeue_put(struct htdx_t *htdx, struct sock_t **i)
{
    if (!i)
    {
        chtd_cry(htdx, "called squeue_put() with NULL arg 2");
        return 0;
    }
    pthread_mutex_lock(&htdx->mx_sq);
    if (htdx->squeue_a == NULL)
    {   /* is empty */
        htdx->squeue_a = htdx->squeue;
        htdx->squeue_z = htdx->squeue;
        htdx->squeue->sock = *i;
    }
    else
    {   /* not empty */
        while (htdx->squeue_z->next == htdx->squeue_a)
        {   /* is full */
            htdx->cv_sq_get_wait = 1;
            pthread_cond_wait(&htdx->cv_sq_get, &htdx->mx_sq);
            htdx->cv_sq_get_wait = 0;
            if (htdx->status != CHTD_RUNNING)
            {
                pthread_mutex_unlock(&htdx->mx_sq);
                return 0;
            }
        }
        htdx->squeue_z = htdx->squeue_z->next;
        htdx->squeue_z->sock = *i;
    }
    pthread_mutex_unlock(&htdx->mx_sq);
    if (htdx->cv_sq_put_wait)
    {
        pthread_cond_signal(&htdx->cv_sq_put);
    }
    return 1;
}


int
squeue_get(struct htdx_t *htdx, struct sock_t **o)
{
    if (!o)
    {
        chtd_cry(htdx, "called squeue_get() with NULL arg 2");
        return 0;
    }
    pthread_mutex_lock(&htdx->mx_sq);
    while (htdx->squeue_a == NULL)
    {   /* is empty */
        htdx->cv_sq_put_wait = 1;
        pthread_cond_wait(&htdx->cv_sq_put, &htdx->mx_sq);
        htdx->cv_sq_put_wait = 0;
        if (htdx->status != CHTD_RUNNING)
        {
            pthread_mutex_unlock(&htdx->mx_sq);
            return 0;
        }
    }
    *o = htdx->squeue_a->sock;
    if (htdx->squeue_a == htdx->squeue_z)
    {   /* only one */
        htdx->squeue_a = NULL;
        htdx->squeue_z = NULL;
    }
    else
    {
        htdx->squeue_a = htdx->squeue_a->next;
    }
    pthread_mutex_unlock(&htdx->mx_sq);
    if (htdx->cv_sq_get_wait)
    {
        pthread_cond_signal(&htdx->cv_sq_get);
    }
    return 1;
}
