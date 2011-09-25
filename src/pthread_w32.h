
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_PTW32_H
#define CHTD_PTW32_H


/*
[ pthread
*/
/*
#define PTW32_STATIC_LIB
#include "pthread.h"

typedef struct
{
#ifdef _UWIN
    DWORD dummy[5];
#endif
    DWORD thread;
    HANDLE threadH;
} ptw32_thread_t;
*/
/*
]
*/


typedef HANDLE pthread_t;
typedef HANDLE pthread_mutex_t;
typedef struct {
    HANDLE signal, broadcast;
} pthread_cond_t;


struct timespec
{
    long tv_nsec;
    long tv_sec;
};


pthread_t
pthread_self(void);


int
pthread_create(pthread_t *, void *, void (*)(void *), void *);


int
pthread_mutex_init(pthread_mutex_t *, void *);


int
pthread_mutex_destroy(pthread_mutex_t *mutex);


int
pthread_mutex_lock(pthread_mutex_t *);


int
pthread_mutex_unlock(pthread_mutex_t *);


int
pthread_cond_init(pthread_cond_t *, const void *);


int
pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);


int
pthread_cond_signal(pthread_cond_t *);


int
pthread_cond_broadcast(pthread_cond_t *);


int
pthread_cond_destroy(pthread_cond_t *);


#endif
