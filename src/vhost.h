
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_VHOST_H
#define CHTD_VHOST_H


#include "chtd.h"


struct vhost_t
{
    char *host;
    char *root;
    char *conf;
    char *real_root;
    struct vhost_t *prev;
    struct vhost_t *next;
};


int
vhost_proc(struct reqs_t *, struct vhost_t *);


void
free_vhosts(struct htdx_t *);


void
vhosts_add(struct htdx_t *, char *, char *, char *, char *);


int
vhosts_del(struct htdx_t *, struct vhost_t *);


struct vhost_t *
chtd_get_vhost(struct htdx_t *, char *);


int
chtd_set_vhost(struct htdx_t *, char *, char *, char *);


struct vhost_t *
chtd_vhost_match(struct reqs_t *);


#endif
