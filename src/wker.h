
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
        WK_DEAD, /* �������߳�û�д��� */
        WK_IDEL, /* ���У��߳��Ѿ����� */
        WK_BUSY, /* æµ�����ڴ������� */
        WK_KEEP  /* ���֣�"Keep-Alive" */
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
