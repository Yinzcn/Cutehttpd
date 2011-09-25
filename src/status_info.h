
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_STATUS_INFO_H
#define CHTD_STATUS_INFO_H


#include "cutehttpd.h"


enum inf_fmt_t
{
    FMT_TEXT = 0,
    FMT_HTML
};


char *
chtd_get_status_info(struct htdx_t *, char *);


#endif
