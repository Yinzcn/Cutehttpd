
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"


#ifdef _WIN32
    #define SHUT_RD   SD_RECEIVE
    #define SHUT_WR   SD_SEND
    #define SHUT_RDWR SD_BOTH
#else
    #define closesocket(s) close(s)
    #define SOCKET int
#endif


struct usa_t   /* unified socket address */
{
    int len;
    union {
        struct sockaddr sa;
        struct sockaddr_in sin;
    } u;
};


struct sock_t
{
    SOCKET socket;
    struct usa_t lsa;
    struct usa_t rsa;
};


void
sock_set_non_blocking(struct sock_t *);


void
set_sock_timeout(struct sock_t *, int, int);
#define sock_set_send_timeout(s, t) set_sock_timeout(s, SO_SNDTIMEO, t);
#define sock_set_recv_timeout(s, t) set_sock_timeout(s, SO_RCVTIMEO, t);


void
sock_close(struct sock_t *);
