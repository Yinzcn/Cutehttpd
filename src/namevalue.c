
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "namevalue.h"


int
namevalues_add(struct namevalue_t **nvs, char *n, int nl, char *v, int vl)
{
    if (n == NULL || v == NULL || nl <= 0 || vl <= 0)
    {
        return 0;
    }
    struct namevalue_t *nv;
    nv = calloc(1, sizeof(struct namevalue_t) + nl + 1 + vl + 1);
    nv->n  = (char *)nv + sizeof(struct namevalue_t);
    nv->v = nv->n + nl + 1;
    memcpy(nv->n, n, nl);
    memcpy(nv->v, v, vl);
    if (*nvs)
    {
        nv->prev = (*nvs)->prev;
        nv->next = (*nvs);
        nv->prev->next = nv;
        nv->next->prev = nv;
    }
    else
    {
        nv->prev = nv;
        nv->next = nv;
        *nvs = nv;
    }
    return 1;
}


int
namevalues_del(struct namevalue_t **nvs, struct namevalue_t *nv)
{
    if (*nvs == NULL || nv == NULL)
    {
        return 0;
    }
    if (nv->next == nv)   /* only one */
    {
        *nvs = NULL;
    }
    else
    {
        if (nv == *nvs)   /* the first */
        {
            *nvs = nv->next;
        }
        nv->prev->next = nv->next;
        nv->next->prev = nv->prev;
    }
    free(nv);
    return 1;
}


struct namevalue_t *
namevalues_get(struct namevalue_t **nvs, char *n)
{
    if (*nvs == NULL)
    {
        return NULL;
    }
    struct namevalue_t *curr, *last;
    curr = *nvs;
    last = curr->prev;
    while (1)
    {
        if (strcasecmp(curr->n, n) == 0)
        {
            return curr;
        }
        if (curr == last)
        {
            return NULL;
        }
        curr = curr->next;
    }
    return NULL;
}


char *
namevalues_get_value(struct namevalue_t **nvs, char *n)
{
    if (*nvs == NULL)
    {
        return NULL;
    }
    struct namevalue_t *nv = namevalues_get(nvs, n);
    if (!nv)
    {
        return NULL;
    }
    return nv->v;
}


void
namevalues_destroy(struct namevalue_t **nvs)
{
    if (*nvs == NULL)
    {
        return;
    }
    struct namevalue_t *curr, *next, *last;
    curr = *nvs;
    last = curr->prev;
    while (1)
    {
        next = curr->next;
        free(curr);
        if (curr == last)
        {
            return;
        }
        curr = next;
    }
}
