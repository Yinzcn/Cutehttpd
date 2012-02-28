
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#ifndef CHTD_BASE_H
#define CHTD_BASE_H


#ifdef _WIN32
    #define x_msleep(ms) Sleep(ms)
    #define strcasecmp(a,b) stricmp(a,b)
    #define snprintf _snprintf
#else
    #define x_msleep(ms) usleep(ms * 1000)
#endif


#define str2equ(s, c0, c1)                                                     \
  ( s[0] == c0 && s[1] == c1 )

#define str3equ(s, c0, c1, c2)                                                 \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 )

#define str4equ(s, c0, c1, c2, c3)                                             \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 )

#define str5equ(s, c0, c1, c2, c3, c4)                                         \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 )

#define str6equ(s, c0, c1, c2, c3, c4, c5)                                     \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4         \
    && s[5] == c5 )

#define str7equ(s, c0, c1, c2, c3, c4, c5, c6)                                 \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4         \
    && s[5] == c5 && s[6] == c6 )

#define str8equ(s, c0, c1, c2, c3, c4, c5, c6, c7)                             \
  ( s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4         \
    && s[5] == c5 && s[6] == c6 && s[7] == c7 )


#ifdef HAVE_STRLWR
#define x_strlwr strlwr
#else
char *
x_strlwr(char *);
#endif


#ifdef HAVE_STRNDUP
#define x_strndup strndup
#else
char *
x_strndup(char *, int);
#endif


#ifdef HAVE_REALPATH
#define x_realpath realpath
#else
char *
x_realpath(char *, char *);
#endif


void *
x_memdup(void *, int);


char *
x_basename(char *);


char *
x_ext_name(char *);


char *
x_nowstr(void);


int
substr_count(char *, char *);


char *
str_replace(char *, char *, char *);


int
is_file(char *);


int
is_dir(char *);


int
is_absolute_path(char *);


void
path_tidy(char *);


void
url_decode(char *, char *);


void
url_encode(char *, char *, int);


int
file_put(char *filename, char *, int);


#endif
