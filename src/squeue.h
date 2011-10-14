
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_SQUEUE_H
#define CHTD_SQUEUE_H


#include "chtd.h"


struct squeue_t
{
    struct sock_t    sock;
    struct squeue_t *prev;
    struct squeue_t *next;
};


int
init_squeue(struct htdx_t *);


void
free_squeue(struct htdx_t *);


int
squeue_put(struct htdx_t *, struct sock_t *);


int
squeue_get(struct htdx_t *, struct sock_t *);


#endif
