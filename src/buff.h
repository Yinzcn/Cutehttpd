
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_BUFF_H
#define CHTD_BUFF_H


struct bufx_blks_t
{
    int size;
    int used;
    char *data;
    struct bufx_blks_t *prev;
    struct bufx_blks_t *next;
};


struct bufx_t
{
    int used;
    int base;
    int toplimit;
    struct bufx_blks_t *blks;
};


struct bufx_t *
bufx_new(int, int);


void
bufx_del(struct bufx_t *);


int
bufx_put(struct bufx_t *, char *, int);


int
bufx_put_str(struct bufx_t *, char *);


int
bufx_get(struct bufx_t *, char *, int);


void
bufx_get_each(struct bufx_t *, void *, void *);


char *
bufx_link(struct bufx_t *);


int
bufx_get_used(struct bufx_t *);


#ifdef DEBUG
void
bufx_debug(struct bufx_t *);
#endif


#endif
