
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "base.h"


char *
chd_strlwr(char *s)
{
    char *p;
    for (p=s; *p; p++)
    {
        if (*p >= 'A' && *p <= 'Z')
        {
            *p = *p - 'A' + 'a';
        }
    }
    return s;
}


int
substr_count(char *str, char *sub)
{
    if (!str || !sub)
    {
        return 0;
    }
    int l = strlen(sub);
    int n = 0;
    char *p = str;
    while ((p = strstr(p, sub)))
    {
        n++;
        p += l;
    }
    return n;
}


/*
f: find
r: replacewith
s: subject
*/
char *
str_replace(char *f, char *r, char *s)
{
    int fl  = strlen(f);
    int rl = strlen(r);
    int sl = strlen(s);

    int n = substr_count(s, f);

    char *bf = calloc(sl + (rl - fl) * n + 1, sizeof(char));
    if (!bf)
    {
        return NULL;
    }

    char *p1, *p2;
    char *p3 = bf;
    p1 = s;
    while ((p2 = strstr(p1, f)))
    {
        n = p2 - p1;
        memcpy(p3, p1, n);
        p3 += n;
        memcpy(p3, r, rl);
        p3 += rl;
        p1 = p2 + fl;
    }
    strcpy(p3, p1);
    return bf;
}


void *
memdup(void *m, int n)
{
    if (!m)
    {
        return NULL;
    }
    char *p = calloc(n, sizeof(char));
    if (!p)
    {
        return NULL;
    }
    memcpy(p, m, n);
    return p;
}


#ifdef WIN32
char *
strndup(char *s, int n)
{
    if (!s)
    {
        return NULL;
    }
    int l = strlen(s);
    if (n > l)
    {
        n = l;
    }
    char *p = calloc(n + 1, sizeof(char));
    if (!p)
    {
        return NULL;
    }
    memcpy(p, s, n);
    return p;
}
#endif


int
is_file(char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        return 0;
    }
    fclose(f);
    return 1;
}


int
is_dir(char *path)
{
    DIR *d = opendir(path);
    if (!d)
    {
        return 0;
    }
    closedir(d);
    return 1;
}


int
is_absolute_path(char *path)
{
    if (path[0] == '/' || path[0] == '\\')
    {
        return 1;
    }
    if (isalpha(path[0]) && path[1] == ':')
    {
        return 1;
    }
    return 0;
}


void
path_tidy(char *path)
{
    if (!path)
    {
        return;
    }
    char *p1, *p2, *p3;

    /*
    [ '\' => '/'
    */
    p1 = path;
    while (*p1)
    {
        if (*p1 == '\\')
        {
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
    while (*p1)
    {
        if (p1[0] == '/' && p1[1] == '/')
        {
            p2 = p1 + 1;
            p3 = p2;
            while (*p2 == '/')
            {
                p2++;
            }
            while (*p3)
            {
                *p3++ = *p2++;
            }
        }
        p1++;
    }
    /*
    ]
    */

    p1 = path;
    while (*p1)
    {
        if (*p1 == '.')
        {
            p2 = p1 + 1;
            while (*p2 == '.')
            {
                p2++;
            }
            if (*p2 != '/' && *p2)
            {
                p1++;
                continue;
            }
            if (p1 != path)
            {
                p1--;
                if (*p1 != '/')
                {
                    p1 = p2;
                    continue;
                }
                if (p2 - p1 > 2)
                {
                    while (*p1 == '/' && p1 > path)
                    {
                        p1--;
                    }
                    while (*p1 != '/' && p1 > path)
                    {
                        p1--;
                    }
                    if (p1 == path && isalpha(p1[0]) && p1[1] == ':')
                    {
                        p1 += 2;
                    }
                }
                p1++;
            }
            while (*p2 == '/')
            {
                p2++;
            }
            /*
            if (*p2) {
                p2--;
            }
            */
            p3 = p1;
            while (*p2)
            {
                *p3++ = *p2++;
            }
            *p3 = '\0';
            continue;
        }
        p1++;
    }

    if (!*path)
    {
        path[0] = '/';
        path[1] = '\0';
    }
}


void
real_path(char *s, char *d)
{
    *d = 0;
    if (is_absolute_path(s))
    {
        strcat(d, s);
    }
    else
    {
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
    do
    {
        if (*s == '%')
        {
            c = s[1];
            if (c >= '0' && c <= '9')
            {
                c -= '0';
            }
            else if (c >= 'A' && c <= 'F')
            {
                c += 10 - 'A';
            }
            else if (c >= 'a' && c <= 'f')
            {
                c += 10 - 'a';
            }
            else
            {
                *d++ = *s++;
                if (!*s)
                {
                    break;
                }
                continue;
            }
            *d = c << 4;
            c = s[2];
            if (c >= '0' && c <= '9')
            {
                c -= '0';
            }
            else if (c >= 'A' && c <= 'F')
            {
                c += 10 - 'A';
            }
            else if (c >= 'a' && c <= 'f')
            {
                c += 10 - 'a';
            }
            else
            {
                *d++ = *s++;
                *d++ = *s++;
                if (!*s)
                {
                    break;
                }
                continue;
            }
            *d += c;
            s += 3;
            d += 1;
        }
        else if (*s == '+')
        {
            *d++ = ' ';
            s++;
        }
        else
        {
            *d++ = *s++;
        }
    }
    while (*s);
}


void
url_encode(char *s, char *d, int max)
{
    static unsigned char hexchars[] = "0123456789ABCDEF";
    register unsigned char c;
    while (*s && max)
    {
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
                && max >= 3)
        {
            *d++ = '%';
            *d++ = hexchars[c >> 4];
            *d++ = hexchars[c & 15];
            max -= 3;
        }
        else
        {
            *d++ = c;
            max--;
        }
    }
    *d = *s;
}


char *
get_file_extname(char *filename)
{
    if (!filename)
    {
        return "";
    }
    char *p;
    p = strrchr(filename, '.');
    if (p)
    {
        return p++;
    }
    return "";
}


int
file_put(char *filename, char *data, int size)
{
    if (!filename || !data)
    {
        return 0;
    }
    if (!size)
    {
        size = strlen(data);
    }
    FILE *pFile;
    pFile = fopen(filename, "ab");
    if (!pFile)
    {
        return 0;
    }
    fseek(pFile, 0, SEEK_END);
    fwrite(data, 1, size, pFile);
    fclose(pFile);
    return 1;
}


int
spawn_process(char *cmdl, char *wdir)
{
#ifdef WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    si.cb  = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (!CreateProcessA(NULL,
        cmdl,
        NULL,
        NULL,
        TRUE,
        CREATE_NEW_PROCESS_GROUP,
        NULL,
        wdir,
        &si,
        &pi))
    {
        return 0;
    }
#endif
    return 1;
}
