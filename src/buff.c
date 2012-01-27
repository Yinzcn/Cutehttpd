
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "buff.h"


struct bufx_t *
bufx_new(int base, int toplimit) {
    struct bufx_t *bufx;
    bufx = calloc(1, sizeof(struct bufx_t));
    if (bufx) {
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
    struct bufx_blks_t *curr, *next, *last;
    if (bufx == NULL) {
        return;
    }
    curr = bufx->blks;
    if (curr) {
        last = curr->prev;
        while (1) {
            next = curr->next;
            free(curr);
            if (curr == last) {
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
    int done = 0;
    if (bufx == NULL || size <= 0) {
        return 0;
    }
    if (bufx->used + size > bufx->toplimit) {
        return 0;
    }
    if (bufx->blks) {
        struct bufx_blks_t *last;
        int left, step;
        last = bufx->blks->prev;
        left = last->size - last->used;
        if (left) {
            step = left > size ? size : left;
            memcpy(last->data + last->used, data, step);
            last->used += step;
            bufx->used += step;
            size -= step;
            done += step;
            data += step;
        }
    }
    if (size > 0) {
        struct bufx_blks_t *blks;
        int sizx = bufx->base;
        while (sizx < size) {
            sizx *= 2;
        }
        blks = calloc(1, sizeof(struct bufx_blks_t) + sizx + 2);
        if (blks == NULL) {
            return done;
        }
        blks->data = (char *)blks + sizeof(struct bufx_blks_t);
        blks->size = sizx;
        blks->used = size;
        memcpy(blks->data, data, size);
        bufx->used += size;
        if (bufx->blks) {
            blks->prev = bufx->blks->prev;
            blks->next = bufx->blks;
            blks->prev->next = blks;
            blks->next->prev = blks;
        } else {
            blks->prev = blks;
            blks->next = blks;
            bufx->blks = blks;
        }
    }
    return done;
}


int
bufx_put_str(struct bufx_t *bufx, char *s)
{
    if (bufx == NULL || s == NULL) {
        return 0;
    }
    return bufx_put(bufx, s, strlen(s));
}


int
bufx_get(struct bufx_t *bufx, char *buff, int need)
{
    if (bufx == NULL || need <= 0) {
        return 0;
    }
    if (bufx->used > 0) {
        int left;
        int step;
        char *bufp = buff;
        struct bufx_blks_t *curr, *next, *last;
        if (need > bufx->used) {
            need = bufx->used;
        }
        left = need;
        curr = bufx->blks;
        last = curr->prev;
        while (1) {
            next = curr->next;
            if (curr->used < left) {
                step = curr->used;
                memcpy(bufp, curr->data, step);
                left -= step;
                bufp += step;
                free(curr);
                curr = next;
                continue;
            } else if (curr->used > left) {
                int blk_left;
                step = left;
                memcpy(bufp, curr->data, step);
                blk_left = curr->used - step;
                memcpy(curr->data, curr->data + step, blk_left);
                curr->size = blk_left;
                curr->used = blk_left;
                curr->prev = last;
                last->next = curr;
                bufx->blks = curr;
                break;
            } else {
                memcpy(bufp, curr->data, left);
                if (curr == last) {
                    bufx->blks = NULL;
                } else {
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
    return 0;
}


void
bufx_get_each(struct bufx_t *bufx, void *func, void *arg1)
{
    if (bufx) {
        struct bufx_blks_t *curr, *next, *last;
        curr = bufx->blks;
        if (curr) {
            last = curr->prev;
            while (1) {
                next = curr->next;
                ((void (*)())func)(arg1, curr->data, curr->used);
                free(curr);
                if (curr == last) {
                    break;
                }
                curr = next;
            }
        }
        bufx->used = 0;
        bufx->blks = NULL;
    }
}


char *
bufx_link(struct bufx_t *bufx)
{
    if (bufx) {
        struct bufx_blks_t *blks;
        int size;
        size = bufx->used;
        if (size == 0) {
            return NULL;
        }
        blks = calloc(1, sizeof(struct bufx_blks_t) + size);
        if (blks == NULL) {
            return NULL;
        }
        blks->data = (char *)blks + sizeof(struct bufx_blks_t);
        blks->size = size;
        blks->used = size;
        blks->prev = blks->next = blks;
        bufx_get(bufx, blks->data, size);
        bufx->blks = blks;
        return blks->data;
    }
    return NULL;
}


int
bufx_get_used(struct bufx_t *bufx)
{
    if (bufx == NULL) {
        return 0;
    }
    return bufx->used;
}
