
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include <string.h>
#include "base64.h"


char *
base64_encode(char *src, int len, int *ret_len)
{
    static char b64e_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
    };

    const unsigned char *s;
    char unsigned *dst, *d;

    if (src == NULL) {
        return NULL;
    }

    if ((unsigned int)((len + 2) / 3) >= (1 << (sizeof(int) * 8 - 2))) {
        if (ret_len != NULL) {
            *ret_len = 0;
        }
        return NULL;
    }

    dst = malloc((len + 2) / 3 * 4 + 1);

    d = dst;
    s = src;

    while (len > 2) {
        *d++ = b64e_table[s[0] >> 2];
        *d++ = b64e_table[((s[0] & 0x03) << 4) | (s[1] >> 4)];
        *d++ = b64e_table[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        *d++ = b64e_table[s[2] & 0x3f];
        s += 3;
        len -= 3;
    }

    if (len) {
        *d++ = b64e_table[s[0] >> 2];
        if (len > 1) {
            *d++ = b64e_table[((s[0] & 0x03) << 4) + (s[1] >> 4)];
            *d++ = b64e_table[((s[1] & 0x0f) << 2)];
            *d++ = '=';
        } else {
            *d++ = b64e_table[((s[0] & 0x03) << 4)];
            *d++ = '=';
            *d++ = '=';
        }
    }

    if (ret_len != NULL) {
        *ret_len = d - dst;
    }

    *d = '\0';
    return dst;
}


char *
base64_decode(char *src, int *ret_len)
{
    static char b64d_table[256] = {
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
        -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
        -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
    };  /* -1: skip space or separator. -2: invalid character */

    char unsigned *s = src;
    char unsigned *dst, *d;
    char ch;
    int i = 0;
    int len;

    if (src == NULL) {
        return NULL;
    }

    len = strlen(src);
    if (len < 4) {
        return NULL;
    }

    dst = malloc(len);
    d = dst;

    while ((ch = *s)) {
        if (ch == '=') {
            break;
        }
        ch = b64d_table[ch];
        if (ch == -1) { /* skip space or separator */
            s++;
            continue;
        } else
        if (ch == -2) {
            free(dst);
            return NULL;
        }
        switch (i % 4) {
            case 0:
                *d = ch << 2;
                break;
            case 1:
                *d++ |= ch >> 4;
                *d = (ch & 0x0f) << 4;
                break;
            case 2:
                *d++ |= ch >> 2;
                *d = (ch & 0x03) << 6;
                break;
            case 3:
                *d++ |= ch;
                break;
        }
        s++;
        i++;
    }

    if (*s++ == '=') {
        while (b64d_table[*s] == -1) {
            s++;
        }
        if ((i % 4 == 0) ||
            (i % 4 == 1) ||
            (i % 4 == 2 && *s != '=') ||
            (i % 4 == 3 && *s == '=')) {
            free(dst);
            return NULL;
        }
    } else
    if (i % 4) {
        free(dst);
        return NULL;
    }

    if (ret_len) {
        *ret_len = d - dst;
    }
    *d = '\0';
    return dst;
}
