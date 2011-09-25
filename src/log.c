
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "log.h"


int
str_time(char *buff)
{
    time_t rawtime;
    struct tm *tm;
    rawtime = time(NULL);
    tm = localtime(&rawtime);
    return snprintf(buff, sizeof(buff) - 2, "[%04d-%02d-%02d %02d:%02d:%02d] ", tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    /*
    SYSTEMTIME st;
    //GetSystemTime(&st);
    GetLocalTime(&st);
    return sprintf(buff, "%02d-%02d %02d:%02d:%02d", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    //return sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d.%04d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    */
}


void
chtd_log(struct htdx_t *htdx, char *f, ...)
{
    pthread_mutex_lock(&htdx->mutex);

    /*
    [ init
    if (htdx->log_Buff == NULL) {
      htdx->log_Buff_size = 4096;
      htdx->log_Buff = mem_alloc(htdx, htdx->log_Buff_size);
    }
    ]
    */

    char b[1024];
    char t[ 128];
    str_time(t);
    int n;
    n = sprintf(b, "[%s] ", t);

    va_list a;
    va_start(a, f);
    n += vsprintf(b + n, f, a);
    va_end(a);

    strcat(b, "\r\n");
#ifdef DEBUG
    printf("%s", b);
#endif
    file_put("chtd.log", b, strlen(b));

    pthread_mutex_unlock(&htdx->mutex);
}


void
chtd_cry_x(struct htdx_t *htdx, char *FILE, int LINE, char *f, ...)
{
    char B[1024];
    char F[1024];

    sprintf(F, "err(%d):%s:%d : %s", errno, FILE, LINE, f);
    va_list a;
    va_start(a, f);
    vsprintf(B, F, a);
    va_end(a);
    if (htdx)
    {
        chtd_log(htdx, B, a);
    }
    else
    {
        char b[1024];
        char t[ 128];
        str_time(t);
        int n;
        n = sprintf(b, "[%s] ", t);
        n += vsprintf(b + n, F, a);

        strcat(b, "\r\n");
        file_put("chtd_cry.log", b, strlen(b));
    }
}
