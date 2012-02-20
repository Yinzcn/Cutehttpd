
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "log.h"


void
chtd_log(struct htdx_t *htdx, char *f, ...)
{
    char b[1024] = { 0 };
    int n;
    va_list a;
    n = snprintf(b, sizeof(b), "[%s] ", x_nowstr());
    va_start(a, f);
    n += vsnprintf(b + n, sizeof(b) - n, f, a);
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
    char B[1024] = { 0 };
    char F[1024] = { 0 };
    int n;
    va_list a;
    va_start(a, f);
    n = snprintf(F, sizeof(F), "err(%d):%s:%d : %s", errno, x_basename(FILE), LINE, f);
    if (htdx) {
        vsnprintf(B, sizeof(B), F, a);
        chtd_log(htdx, B, a);
    } else {
        n = snprintf(B, sizeof(B), "[%s] ", x_nowstr());
        n += vsnprintf(B + n, sizeof(B) - n, F, a);
        strcat(B, "\r\n");
#ifdef DEBUG
        printf("%s", B);
#endif
        file_put("chtd_cry.log", B, strlen(B));
    }
    va_end(a);
}
