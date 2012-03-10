
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


#endif
