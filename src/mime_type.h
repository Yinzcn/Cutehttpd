
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_MIME_TYPE_H
#define CHTD_MIME_TYPE_H


#include "chtd.h"


struct mime_type_t
{
    char *ext_name;
    char *type;
    struct mime_type_t *prev;
    struct mime_type_t *next;
};


void
mime_types_add(struct htdx_t *, char *, char *);


int
mime_types_del(struct htdx_t *, char *);


int
mime_type_assign(struct htdx_t *, char *, char *);


struct mime_type_t *
mime_types_get(struct htdx_t *, char *);


char *
get_mime_type(struct htdx_t *, char *);


void
free_mime_types(struct htdx_t *);


#endif
