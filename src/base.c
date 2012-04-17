
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "base.h"


#ifndef HAVE_STRLWR
char *
x_strlwr(char *s)
{
    unsigned char c;
    do {
        c = *s;
        if (c == 0) {
            break;
        } else
        if (c >= 'A' && c <= 'Z') {
            *s = c - 'A' + 'a';
        }
    } while (++s);
    return s;
}
#endif


#ifndef HAVE_STRNDUP
char *
x_strndup(char *s, int n)
{
    char *z, *r;
    if (s) {
        z = memchr(s, '\0', n);
        if (z) {
            n = z - s;
        }
        r = malloc(n + 1);
        if (r) {
            memcpy(r, s, n);
            r[n] = '\0';
            return r;
        }
    }
    return NULL;
}
#endif


#ifndef HAVE_REALPATH
char *
x_realpath(char *s, char *d)
{
#ifdef _WIN32
    if (_fullpath(d, s, MAX_PATH)) {
        return d;
    }
#endif
    return NULL;
}
#endif


void *
x_memdup(void *m, int n)
{
    if (m) {
        char *r = malloc(n);
        if (r) {
            memcpy(r, m, n);
            return r;
        }
    }
    return NULL;
}


char *
x_basename(char *path)
{
    char *p1 = strrchr(path, '/' );
    char *p2 = strrchr(path, '\\');
    char *p3 = (p1 > p2) ? p1 : p2;
    return p3 ? (p3 + 1) : path;
}


char *
x_ext_name(char *name)
{
    if (name) {
        char *p = strrchr(name, '.');
        if (p) {
            return p++;
        }
    }
    return "";
}


char *
x_nowstr(void)
{
    static char buff[20];
    time_t rawtime = time(NULL);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&rawtime));
    return buff;
}


int
substr_count(char *s, char *f)
{
    if (s && f) {
        int n = 0;
        int l = strlen(f);
        while (1) {
            s = strstr(s, f);
            if (!s) {
                break;
            }
            n++;
            s += l;
        }
        return n;
    }
    return 0;
}


/*
    f: search
    r: replace
    s: subject
*/
char *
str_replace(char *f, char *r, char *s)
{
    int fl = strlen(f);
    int rl = strlen(r);
    int sl = strlen(s);
    int n  = substr_count(s, f);
    char *bf = calloc(sl + n * (rl - fl) + 1, sizeof(char));
    if (bf) {
        char *p1, *p2;
        char *bp = bf;
        p1 = s;
        while (1) {
            p2 = strstr(p1, f);
            if (!p2) {
                break;
            }
            n = p2 - p1;
            memcpy(bp, p1, n);
            bp += n;
            memcpy(bp, r, rl);
            bp += rl;
            p1 = p2 + fl;
        }
        strcpy(bp, p1);
    }
    return bf;
}


/*
int
is_file(char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}
*/


int
is_absolute_path(char *path)
{
    if (path[0] == '/' || path[0] == '\\') {
        return 1;
    }
    if (isalpha(path[0]) && path[1] == ':') {
        return 1;
    }
    return 0;
}


void
path_tidy(char *path)
{
    char *p1, *p2, *p3;

    if (!path) {
        return;
    }

    /*
    [ '\' => '/'
    */
    p1 = path;
    while (*p1) {
        if (*p1 == '\\') {
            *p1 = '/';
        }
        p1++;
    }
    /*
    ]
    */

    /*
    [ "//" => '/'
    */
    p1 = path;
    while (*p1) {
        if (p1[0] == '/' && p1[1] == '/') {
            p2 = p1 + 1;
            p3 = p2;
            while (*p2 == '/') {
                p2++;
            }
            while (*p3) {
                *p3++ = *p2++;
            }
        }
        p1++;
    }
    /*
    ]
    */

    p1 = path;
    while (*p1) {
        if (*p1 == '.') {
            p2 = p1 + 1;
            while (*p2 == '.') {
                p2++;
            }
            if (*p2 != '/' && *p2) {
                p1++;
                continue;
            }
            if (p1 != path) {
                p1--;
                if (*p1 != '/') {
                    p1 = p2;
                    continue;
                }
                if (p2 - p1 > 2) {
                    while (*p1 == '/' && p1 > path) {
                        p1--;
                    }
                    while (*p1 != '/' && p1 > path) {
                        p1--;
                    }
                    if (p1 == path && isalpha(p1[0]) && p1[1] == ':') {
                        p1 += 2;
                    }
                }
                p1++;
            }
            while (*p2 == '/') {
                p2++;
            }
            /*
            if (*p2) {
                p2--;
            }
            */
            p3 = p1;
            while (*p2) {
                *p3++ = *p2++;
            }
            *p3 = '\0';
            continue;
        }
        p1++;
    }

    if (!*path) {
        path[0] = '/';
        path[1] = '\0';
    }
}


void
url_decode(char *s, char *d)
{
    register unsigned char c;
    do {
        if (*s == '%') {
            c = s[1];
            if (c >= '0' && c <= '9') {
                c -= '0';
            } else
            if (c >= 'A' && c <= 'F') {
                c += 10 - 'A';
            } else
            if (c >= 'a' && c <= 'f') {
                c += 10 - 'a';
            } else {
                *d++ = *s++;
                if (!*s) {
                    break;
                }
                continue;
            }
            *d = c << 4;
            c = s[2];
            if (c >= '0' && c <= '9') {
                c -= '0';
            } else
            if (c >= 'A' && c <= 'F') {
                c += 10 - 'A';
            } else
            if (c >= 'a' && c <= 'f') {
                c += 10 - 'a';
            } else {
                *d++ = *s++;
                *d++ = *s++;
                if (!*s) {
                    break;
                }
                continue;
            }
            *d += c;
            s += 3;
            d += 1;
        } else
        if (*s == '+') {
            *d++ = ' ';
            s++;
        } else {
            *d++ = *s++;
        }
    } while (*s);
}


void
url_encode(char *s, char *d, int max)
{
    static unsigned char hex[] = "0123456789ABCDEF";
    register unsigned char c;
    while (*s && max) {
        c = *s++;
        /*
        if (c == ' ') {
            *d++ = '+';
        max--;
        } else
        */
        if (((c < '0' && c != '-' && c != '.')
             || (c < 'A' && c > '9')
             || (c < 'a' && c > 'Z' && c != '_')
             || (c > 'z' || c == ' '))
            && max >= 3) {
            *d++ = '%';
            *d++ = hex[c >> 4];
            *d++ = hex[c & 15];
            max -= 3;
        } else {
            *d++ = c;
            max--;
        }
    }
    *d = *s;
}


char *
file_get(char *filename)
{
    char *filedata = NULL;
    int filesize;
    FILE *pFile = fopen(filename, "rb");
    if (pFile) {
        fseek(pFile, 0, SEEK_END);
        filesize = ftell(pFile);
        rewind(pFile);
        filedata = calloc(filesize + 1, sizeof(char));
        fread(filedata, 1, filesize, pFile);
    }
    return filedata;
}


int
file_add(char *filename, char *data, int size)
{
    FILE *pFile;
    if (!filename || !data) {
        return 0;
    }
    if (!size) {
        size = strlen(data);
    }
    pFile = fopen(filename, "ab");
    if (!pFile) {
        return 0;
    }
    fseek(pFile, 0, SEEK_END);
    fwrite(data, 1, size, pFile);
    fclose(pFile);
    return 1;
}
