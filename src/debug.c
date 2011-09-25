
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifdef DEBUG
#include "cutehttpd.h"


int
chtd_print_uhooks(struct htdx_t *htdx)
{
    int n = 0;
    struct uhook_t *curr, *last;
    curr = htdx->uhooks;
    if (curr)
    {
        last = curr->prev;
        while (1)
        {
            printf("uhook#%d host:%s uri:%s\n", n, curr->host, curr->xuri);
            n++;
            if (curr == last)
            {
                break;
            }
            curr = curr->next;
        }
    }
    return n;
}


int
chtd_print_vhosts(struct htdx_t *htdx)
{
    int n = 0;
    struct vhost_t *Curr, *Last;
    Curr = htdx->vhosts;
    if (Curr)
    {
        Last = Curr->prev;
        while (1)
        {
            /* [ */
            printf("vhost#%d host:%s root:%s conf:%s real_root:%s\n", n, Curr->host, Curr->root, Curr->conf, Curr->real_root);
            n++;
            /* ] */
            if (Curr == Last)
            {
                break;
            }
            Curr = Curr->next;
        }
    }
    else
    {
        printf("no vhost\n");
    }
    return n;
}


#endif
