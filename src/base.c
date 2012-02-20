
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "base.h"


char *
x_basename(char *path)
{
    char *s1 = strrchr(path, '/' );
    char *s2 = strrchr(path, '\\');
    char *s3 = (s1 > s2) ? s1 : s2;
    return s3 ? (s3 + 1) : path;
}


char *
x_ext_name(char *name)
{
    char *p;
    if (!name) {
        return "";
    }
    p = strrchr(name, '.');
    if (p) {
        return p++;
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


char *
x_strlwr(char *s)
{
    char *p;
    for (p = s; *p; p++) {
        if (*p >= 'A' && *p <= 'Z') {
            *p = *p - 'A' + 'a';
        }
    }
    return s;
}


int
substr_count(char *str, char *sub)
{
    if (str && sub) {
        int l = strlen(sub);
        int n = 0;
        char *p = str;
        while ((p = strstr(p, sub))) {
            n++;
            p += l;
        }
        return n;
    }
    return 0;
}


/*
f: find
r: replacewith
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
        char *p3 = bf;
        p1 = s;
        while ((p2 = strstr(p1, f))) {
            n = p2 - p1;
            memcpy(p3, p1, n);
            p3 += n;
            memcpy(p3, r, rl);
            p3 += rl;
            p1 = p2 + fl;
        }
        strcpy(p3, p1);
    }
    return bf;
}


void *
memdup(void *m, int n)
{
    if (m) {
        char *p = calloc(n, sizeof(char));
        if (p) {
            memcpy(p, m, n);
            return p;
        }
    }
    return NULL;
}


#ifndef HAVE_STRNDUP
char *
strndup(char *s, int n)
{
    if (s) {
        int l = strlen(s);
        char *p;
        if (n > l) {
            n = l;
        }
        p = calloc(n + 1, sizeof(char));
        if (p) {
            memcpy(p, s, n);
            return p;
        }
    }
    return NULL;
}
#endif


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
real_path(char *s, char *d)
{
    *d = 0;
    if (is_absolute_path(s)) {
        strcat(d, s);
    } else {
        char w[FILENAME_MAX];
        getcwd(w, FILENAME_MAX);
        strcat(d, w);
        strcat(d, "/");
        strcat(d, s);
    }
    path_tidy(d);
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
    static unsigned char hexchars[] = "0123456789ABCDEF";
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
            *d++ = hexchars[c >> 4];
            *d++ = hexchars[c & 15];
            max -= 3;
        } else {
            *d++ = c;
            max--;
        }
    }
    *d = *s;
}


int
file_put(char *filename, char *data, int size)
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


struct envblk_t {
    int   size;
    int   used;
    char *data;
};


int
envblk_add(struct envblk_t **envblk, char *n, char *v)
{
    int nl;
    int vl;
    struct envblk_t *envb;
    if (!n || !v) {
        return 0;
    }

    nl = strlen(n);
    vl = strlen(v);
    if (nl == 0) {
        return 0;
    }

    envb = *envblk;
    if (envb == NULL) {
        envb = calloc(1, sizeof(struct envblk_t) + 1024);
        envb->size = 1024;
        envb->used = 0;
        envb->data = (char *)(envb + 1);
        *envblk = envb;
    }

    if ((nl + vl) > (envb->size - envb->used - 1)) {
        return 0;
    }

    envb->used += sprintf(envb->data + envb->used, "%s=%s", n, v) + 1;
    return 1;
}


int
spawn_process(struct htdx_t *htdx, char *cmdl, char *wdir)
{
#ifdef _WIN32
    struct envblk_t *envb;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    si.cb  = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;//SW_HIDE;

    envblk_add(&envb, "PATH", getenv("PATH"));
    envblk_add(&envb, "COMSPEC", getenv("COMSPEC"));
    envblk_add(&envb, "SYSTEMROOT", getenv("SYSTEMROOT"));
    if (!CreateProcessA(NULL,
                        cmdl,
                        NULL,
                        NULL,
                        TRUE,
                        CREATE_NEW_PROCESS_GROUP,
                        envb->data,
                        wdir,
                        &si,
                        &pi)) {
        chtd_cry(htdx, "CreateProcessA!");
        return 0;
    }
    chtd_cry(htdx, "CreateProcessA!");
    return 1;
#else
    return 0;
#endif
}
