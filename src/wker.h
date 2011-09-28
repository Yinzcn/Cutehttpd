
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_wker_H
#define CHTD_wker_H


#include "cutehttpd.h"


struct wker_t
{
    int w_id;       /* wker id */
    pthread_t t_id; /* thread id */
    pthread_mutex_t mx_wake;
    pthread_cond_t  cv_wake;
    int nConn;
    int nReqs;
    time_t birthtime;
    enum
    {
        WK_IDEL, /* ���У��߳�û�д��� */
        WK_WAIT, /* ���У��߳��Ѿ����� */
        WK_BUSY, /* æµ�����ڴ������� */
        WK_KEEP, /* ���֣�"Keep-Alive" */
        WK_HUNG,  /* ���𣺼��ոձ��ٻ� */
        WK_UING  /* ���𣺼��ոձ��ٻ� */
    } status;
    char step;
    struct conn_t *conn;
    struct htdx_t *htdx;
    struct wker_t *prev;
    struct wker_t *next;
};


void
init_wkers(struct htdx_t *);


void
free_wkers(struct htdx_t *);


int
get_idel_wker(struct htdx_t *, struct wker_t **);


int
put_idel_wker(struct htdx_t *, struct wker_t **);


int
wker_create_thread(struct wker_t *);


int
wker_wake(struct wker_t *);


int
wker_get_id(struct wker_t *);


void
wker_stat(struct htdx_t *);


#endif
