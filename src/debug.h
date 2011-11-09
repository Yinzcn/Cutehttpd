
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifdef DEBUG
#ifndef CHTD_DEBUG_H
#define CHTD_DEBUG_H


#include "chtd.h"


int
chtd_print_uhooks(struct htdx_t *);


int
chtd_print_vhosts(struct htdx_t *);


void
bufx_debug(struct bufx_t *);


void
dump_namevalues(struct namevalue_t **);


#endif
#endif
