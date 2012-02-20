
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_LOG_H
#define CHTD_LOG_H


#define chtd_cry(...) chtd_cry_x(__FILE__, __LINE__, __VA_ARGS__)


int
str_time(char *);


void
chtd_log(struct htdx_t *, char *, ...);


void
chtd_cry_x(char *, int, struct htdx_t *, char *, ...);


#endif
