
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_LOG_H
#define CHTD_LOG_H


#define chtd_cry(x,a...) chtd_cry_x(x,__FILE__,__LINE__,##a)


int
str_time(char *);


void
chtd_log(struct htdx_t *, char *, ...);


void
chtd_cry_x(struct htdx_t *, char *, int, char *, ...);


#endif
