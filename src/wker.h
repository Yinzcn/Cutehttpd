
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_wker_H
#define CHTD_wker_H


#include "chtd.h"


struct wker_t
{
    int w_id;       /* wker id */
    pthread_t t_id; /* thread id */
    int nConn;
    int nReqs;
    enum
    {
        WK_DEAD, /* 死亡：线程没有创建 */
        WK_IDEL, /* 空闲：线程已经创建 */
        WK_BUSY, /* 忙碌：正在处理请求 */
        WK_KEEP  /* 保持："Keep-Alive" */
    } status;
    struct conn_t *conn;
    struct htdx_t *htdx;
    struct wker_t *prev;
    struct wker_t *next;
};


int
init_wkers(struct htdx_t *);


int
free_wkers(struct htdx_t *);


int
wker_create_thread(struct wker_t *);


int
wker_get_id(struct wker_t *);


void
wker_stat(struct htdx_t *);


#endif
