
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "log.h"


void
chtd_log(struct htdx_t *htdx, char *f, ...)
{
    char b[1024];
    int n;
    va_list a;
    n = sprintf(b, "[%s] ", x_nowstr());
    va_start(a, f);
    n += vsprintf(b + n, f, a);
    va_end(a);

    strcat(b, "\r\n");

#ifdef DEBUG
    pthread_mutex_lock(&htdx->mutex);
    printf("%s", b);
    pthread_mutex_unlock(&htdx->mutex);
#endif
    file_put("chtd.log", b, strlen(b));
}


void
chtd_cry_x(char *FILE, int LINE, struct htdx_t *htdx, char *f, ...)
{
    char B[1024];
    char F[1024];
    va_list a;
    va_start(a, f);
    sprintf(F, "err(%d):%s:%d : %s", errno, x_basename(FILE), LINE, f);
    if (htdx) {
        vsprintf(B, F, a);
        chtd_log(htdx, B, a);
    } else {
        char b[1024];
        int n;
        n = sprintf(b, "[%s] ", x_nowstr());
        n += vsprintf(b + n, F, a);
        strcat(b, "\r\n");
#ifdef DEBUG
        printf("%s", b);
#endif
        file_put("chtd_cry.log", b, strlen(b));
    }
    va_end(a);
}
