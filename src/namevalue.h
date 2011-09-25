
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_NAMEVALUE_H
#define CHTD_NAMEVALUE_H


struct namevalue_t
{
    char *n; /* name  */
    char *v; /* value */
    struct namevalue_t *prev;
    struct namevalue_t *next;
};


int
namevalues_add(struct namevalue_t **, char *, int, char *, int);


int
namevalues_del(struct namevalue_t **, struct namevalue_t *);


struct namevalue_t *
namevalues_get(struct namevalue_t **, char *);


char *
namevalues_get_value(struct namevalue_t **, char *);


void
namevalues_destroy(struct namevalue_t **);


#endif
