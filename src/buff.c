
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "buff.h"


struct bufx_t *
bufx_new(int base, int toplimit)
{
    struct bufx_t *bufx;
    bufx = calloc(1, sizeof(struct bufx_t));
    if (bufx)
    {
        bufx->used = 0;
        bufx->base = base;
        bufx->toplimit = toplimit;
        bufx->blks = NULL;
    }
    return bufx;
}


void
bufx_del(struct bufx_t *bufx)
{
    if (bufx == NULL)
    {
        return;
    }
    struct bufx_blks_t *curr, *next, *last;
    curr = bufx->blks;
    if (curr)
    {
        last = curr->prev;
        while (1)
        {
            next = curr->next;
            free(curr);
            if (curr == last)
            {
                break;
            }
            curr = next;
        }
    }
    free(bufx);
}


int
bufx_put(struct bufx_t *bufx, char *data, int size)
{
    if (bufx == NULL || size <= 0)
    {
        return 0;
    }
    if (bufx->used + size > bufx->toplimit)
    {
        return 0;
    }
    if (bufx->blks)
    {
        struct bufx_blks_t *last;
        int left, step;
        last = bufx->blks->prev;
        left = last->size - last->used;
        if (left)
        {
            step = left > size ? size : left;
            memcpy(last->data + last->used, data, step);
            last->used += step;
            bufx->used += step;
            size       -= step;
            if (size == 0)
            {
                return 1;
            }
            data += step;
        }
    }
    int sizex = bufx->base;
    while (sizex < size)
    {
        sizex *= 2;
    }
    struct bufx_blks_t *blks;
    blks = calloc(1, sizeof(struct bufx_blks_t) + sizex + 1);
    blks->data = (char *)blks + sizeof(struct bufx_blks_t);
    blks->size = sizex;
    blks->used = size;
    memcpy(blks->data, data, size);
    bufx->used += size;
    if (bufx->blks)
    {
        blks->prev = bufx->blks->prev;
        blks->next = bufx->blks;
        blks->prev->next = blks;
        blks->next->prev = blks;
    }
    else
    {
        blks->prev = blks;
        blks->next = blks;
        bufx->blks = blks;
    }
    return 1;
}


int
bufx_put_str(struct bufx_t *bufx, char *s)
{
    return bufx_put(bufx, s, strlen(s));
}


int
bufx_get(struct bufx_t *bufx, char *buff, int need)
{
    if (bufx == NULL || need <= 0)
    {
        return 0;
    }
    if (bufx->used == 0)
    {
        return 0;
    }
    if (need > bufx->used)
    {
        need = bufx->used;
    }
    int left = need;
    int step;
    char *bptr = buff;
    struct bufx_blks_t *curr, *next, *last;
    curr = bufx->blks;
    last = curr->prev;
    while (1)
    {
        next = curr->next;
        if (curr->used < left)
        {
            step = curr->used;
            memcpy(bptr, curr->data, step);
            left -= step;
            bptr += step;
            free(curr);
            curr = next;
            continue;
        }
        else
        if (curr->used > left)
        {
            step = left;
            memcpy(bptr, curr->data, step);
            int blk_left = curr->used - step;
            memcpy(curr->data, curr->data + step, blk_left);
            curr->size = blk_left;
            curr->used = blk_left;
            curr->prev = last;
            last->next = curr;
            bufx->blks = curr;
            break;
        }
        else
        {
            memcpy(bptr, curr->data, left);
            if (curr == last)
            {
                bufx->blks = NULL;
            }
            else
            {
                next->prev = last;
                last->next = next;
                bufx->blks = next;
            }
            free(curr);
            break;
        }
    }
    bufx->used -= need;
    return need;
}


void
bufx_get_each(struct bufx_t *bufx, void *(*func)(), void *arg1)
{
    if (bufx == NULL)
    {
        return;
    }
    struct bufx_blks_t *curr, *next, *last;
    curr = bufx->blks;
    if (curr)
    {
        last = curr->prev;
        while (1)
        {
            next = curr->next;
            (void*)func(arg1, curr->data, curr->used);
            free(curr);
            if (curr == last)
            {
                break;
            }
            curr = next;
        }
    }
    bufx->used = 0;
    bufx->blks = NULL;
}


char *
bufx_link(struct bufx_t *bufx)
{
    if (bufx == NULL)
    {
        return NULL;
    }
    int size;
    size = bufx->used;
    if (size == 0)
    {
        return NULL;
    }
    struct bufx_blks_t *blks;
    blks = calloc(1, sizeof(struct bufx_blks_t) + size);
    blks->data = (char *)blks + sizeof(struct bufx_blks_t);
    blks->size = size;
    blks->used = size;
    blks->prev = blks->next = blks;
    bufx_get(bufx, blks->data, size);
    bufx->blks = blks;
    return blks->data;
}


int
bufx_get_used(struct bufx_t *bufx)
{
    if (!bufx)
    {
        return 0;
    }
    return bufx->used;
}


#ifdef DEBUG
void
bufx_debug(struct bufx_t *bufx)
{
    if (bufx == 0)
    {
        return;
    }
    printf("bufx->used=%d\n", bufx->used);
    struct bufx_blks_t *curr, *last;
    curr = bufx->blks;
    if (curr)
    {
        last = curr->prev;
        while (1)
        {
            printf("curr: size=%d used=%d data=[%s]\n", curr->size, curr->used, curr->data);
            if (curr == last)
            {
                break;
            }
            curr = curr->next;
        }
    }
}
#endif
