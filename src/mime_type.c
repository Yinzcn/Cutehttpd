
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "mime_type.h"


void
mime_types_add(struct htdx_t *htdx, char *ext_name, char *type)
{
    struct mime_type_t *mime_type;
    char buff[16] = "\0";
    strncat(buff, ext_name, 15);
    x_strlwr(buff);
    mime_type = calloc(1, sizeof(struct mime_type_t));
    mime_type->ext_name = strdup(buff);
    mime_type->type     = strdup(type);
    if (htdx->mime_types) {
        mime_type->prev = htdx->mime_types->prev;
        mime_type->next = htdx->mime_types;
        mime_type->prev->next = mime_type;
        mime_type->next->prev = mime_type;
    } else {
        mime_type->prev  = mime_type;
        mime_type->next  = mime_type;
        htdx->mime_types = mime_type;
    }
}


int
mime_types_del(struct htdx_t *htdx, char *ext_name)
{
    struct mime_type_t *mime_type;
    mime_type = mime_types_get(htdx, ext_name);
    if (mime_type) {
        if (mime_type == mime_type->next) { // only one
            htdx->mime_types = NULL;
        } else {
            if (mime_type == htdx->mime_types) { // the first
                htdx->mime_types = mime_type->next;
            }
            mime_type->prev->next = mime_type->next;
            mime_type->next->prev = mime_type->prev;
        }
        free(mime_type->ext_name);
        free(mime_type->type);
        free(mime_type);
        return 1;
    } else {
        return 0;
    }
}


int
mime_type_assign(struct htdx_t *htdx, char *ext_name, char *type)
{
    if (!strlen(ext_name)) {
        return 0;
    }
    if (strlen(type)) {
        struct mime_type_t *mime_type;
        mime_type = mime_types_get(htdx, ext_name);
        if (mime_type) { /* to update */
            free(mime_type->type);
            mime_type->type = x_strlwr(strdup(type));
            return 1;
        } else { /* new */
            mime_types_add(htdx, ext_name, type);
            return 1;
        }
    } else { /* to delete */
        return mime_types_del(htdx, ext_name);
    }
}


struct mime_type_t *
mime_types_get(struct htdx_t *htdx, char *ext_name) {
    struct mime_type_t *curr, *last;
    curr = htdx->mime_types;
    if (curr) {
        char buff[16] = "\0";
        strncat(buff, ext_name, 15);
        x_strlwr(buff);
        last = curr->prev;
        while (1) {
            if (strcmp(curr->ext_name, buff) == 0) {
                return curr;
            }
            if (curr == last) {
                return NULL;
            }
            curr = curr->next;
        }
    }
    return NULL;
}


char *
get_mime_type(struct htdx_t *htdx, char *ext_name)
{
    struct mime_type_t *mime_type;
    mime_type = mime_types_get(htdx, ext_name);
    if (mime_type) {
        htdx->mime_types = mime_type;
        return mime_type->type;
    } else {
        mime_type = mime_types_get(htdx, "*");
        if (mime_type) {
            return  mime_type->type;
        }
    }
    return "";
}


void
free_mime_types(struct htdx_t *htdx)
{
    struct mime_type_t *curr, *next, *last;
    if (!htdx->mime_types) {
        return;
    }
    curr = htdx->mime_types;
    last = curr->prev;
    while (1) {
        next = curr->next;
        free(curr->ext_name);
        free(curr->type);
        free(curr);
        if (curr == last) {
            break;
        }
        curr = next;
    }
}
