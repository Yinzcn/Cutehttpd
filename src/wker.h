
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
    pthread_mutex_t mx_wker;
    pthread_cond_t  cv_wake;
    int nConn;
    int nReqs;
    enum
    {
        WK_DEAD, /* �������߳�û�д��� */
        WK_IDEL, /* ���У��߳��Ѿ����� */
        WK_HUNG, /* ���𣺼��ոձ��ٻ� */
        WK_WKUP, /* ���ѣ����ڻ���״̬ */
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
get_idel_wker(struct htdx_t *, struct wker_t **);


int
put_idel_wker(struct htdx_t *, struct wker_t **);


int
wker_create_thread(struct wker_t *);


int
wker_wakeup(struct wker_t *);


int
wker_get_id(struct wker_t *);


void
wker_stat(struct htdx_t *);


#endif
