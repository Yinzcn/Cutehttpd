
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_UHOOK_H
#define CHTD_UHOOK_H


#include "cutehttpd.h"


struct uhook_t
{
    char *host;
    char *xuri;
    void *(*func)();
    void *pPcre;
    struct uhook_t *prev;
    struct uhook_t *next;
};


struct uhook_t *
uhooks_add(struct htdx_t *, char *, char *, void *, void *);


struct uhook_t *
uhooks_del(struct htdx_t *, struct uhook_t *, struct uhook_t *);


struct uhook_t *
uhooks_get(struct uhook_t *, char *, char *);


int
free_uhooks(struct htdx_t *);


struct uhook_t *
chtd_get_uhook(struct htdx_t *, char *, char *);


int
chtd_set_uhook(struct htdx_t *, char *, char *, void *);


struct uhook_t *
chtd_uhook_match(struct reqs_t *);


#endif
