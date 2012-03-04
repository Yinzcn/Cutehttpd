
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "namevalue.h"
#include "parse_header.h"


#define is_valid_name_char(c) \
  ( (c == '-') || (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') )


int
parse_header(struct namevalue_t **nvs, char *hstr)
{
    char *n_a = NULL;
    char *n_z = NULL;
    char *v_a = NULL;
    char *v_z = NULL;
    int   n_l;
    int   v_l;
    char *p = hstr;
    int count = 0;
    int loop = 1;
    enum {
        sw_name_bot,
        sw_name_eot,
        sw_value_bot,
        sw_value_eot,
        sw_field_done,
        sw_next_line,
    } state = sw_name_bot;

    do {
        switch (state) {
        /* name BOT */
        case sw_name_bot:
            if (*p == CR || *p == LF) {
                loop = 0;
                break;
            }
            if (is_valid_name_char(*p)) {
                n_a = p;
                state = sw_name_eot;
            } else {
                state = sw_next_line;
            }
            break;

        /* name EOT */
        case sw_name_eot:
            while (is_valid_name_char(*p)) {
                p++;
            }
            n_z = p;
            /* space after name*/
            while (*p == SP || *p == HT) {
                p++;
            }
            if (*p == ':') {
                p++;
                state = sw_value_bot;
            } else {
                state = sw_next_line;
            }
            break;

        /* value BOT */
        case sw_value_bot:
            /* space before value */
            while (*p == SP || *p == HT) {
                p++;
            }
            v_a = p;
            state = sw_value_eot;
            break;

        /* value EOT */
        case sw_value_eot:
            while (*p != LF && *p) {
                p++;
            }
            if ((p[0] == LF) && (p[1] == SP || p[1] == HT)) {
            /* is multi lines value */
                p++;
                state = sw_value_eot;
                break;
            }
            /* space after value */
            do {
                p--;
            } while (*p == SP || *p == HT || *p == CR);
            p++;
            v_z = p;
            state = sw_field_done;
            break;

        case sw_field_done:
            n_l = n_z - n_a;
            v_l = v_z - v_a;
            if (n_l > 0 && v_l > 0) {
                namevalues_add(nvs, n_a, n_l, v_a, v_l);
                count++;
            }
            state = sw_next_line;
            break;

        case sw_next_line:
            while (*p != LF && *p) {
                p++;
            }
            if (p[0] && p[1]) {
                p++;
                state = sw_name_bot;
                break;
            }
            loop = 0;
            break;
/*
        default:
            loop = 0;
            break;
*/
        }
    } while (loop);
    return count;
}
