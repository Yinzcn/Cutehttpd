
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"


#ifndef W32_ENVBLK
#define W32_ENVBLK


struct envblk_t
{
    int buflen;
    int blklen;
    char *blkbuf;
};


struct envblk_t *
envblk_add(struct envblk_t *envblk, char *n, char *v)
{
    int nl;
    int vl;
    if (!n || !v) {
        return 0;
    }
    nl = strlen(n);
    vl = strlen(v);
    if (!nl || !vl) {
        return 0;
    }
    if (envblk->blkbuf == NULL) {
        envblk->blkbuf = calloc(1024, sizeof(char));
        envblk->buflen = 1024;
        envblk->blklen = 0;
    }
    while ((envblk->buflen - envblk->blklen) < (nl + 1 + vl + 2)) {
        envblk->blkbuf = realloc(envblk->blkbuf, envblk->buflen + 1024);
        envblk->buflen += 1024;
    }
    memcpy(envblk->blkbuf + envblk->blklen, n, nl);
    envblk->blklen += nl;
    envblk->blkbuf[envblk->blklen++] = '=';
    memcpy(envblk->blkbuf + envblk->blklen, v, vl);
    envblk->blklen += vl;
    envblk->blkbuf[envblk->blklen++] = '\0';
    envblk->blkbuf[envblk->blklen]   = '\0';
    return envblk;
}


struct envblk_t *
envblk_add_x(struct envblk_t *envblk, char *n, char *f, ...)
{
    char b[1024] = { 0 };
    int c;
    va_list v;
    va_start(v, f);
    c = vsnprintf(b, sizeof(b), f, v);
    va_end(v);
    if (!c) {
        return envblk;
    }
    return envblk_add(envblk, n, b);
}


#endif
