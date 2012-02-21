
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_BASE_H
#define CHTD_BASE_H


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


char *
x_basename(char *);


char *
x_ext_name(char *);


char *
x_nowstr(void);


char *
x_strlwr(char *);


int
substr_count(char *, char *);


char *
str_replace(char *, char *, char *);


void *
x_memdup(void *, int);


#ifdef HAVE_STRNDUP
#define x_strndup strndup
#else
char *
x_strndup(char *, int);
#endif


int
is_file(char *);


int
is_dir(char *);


int
is_absolute_path(char *);


void
path_tidy(char *);


void
real_path(char *, char *);


void
url_decode(char *, char *);


void
url_encode(char *, char *, int);


int
file_put(char *filename, char *, int);


#endif
