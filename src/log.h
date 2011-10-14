
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_LOG_H
#define CHTD_LOG_H

#ifdef _MSC_VER
#define chtd_cry(x,f,...) chtd_cry_x(x, __FILE__, __LINE__, f, __VA_ARGS__)
#else
#define chtd_cry(x,vf...) chtd_cry_x(x, __FILE__, __LINE__, ##vf)
#endif


int
str_time(char *);


void
chtd_log(struct htdx_t *, char *, ...);


void
chtd_cry_x(struct htdx_t *, char *, int, char *, ...);


#endif
