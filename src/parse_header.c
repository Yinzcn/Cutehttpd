
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "namevalue.h"
#include "parse_header.h"


/*

GET /hello/Yinz.php?i=Piao&f=China HTTP/1.1
Host: phpnow.org
User-Agent: Firefox

*/


#define isvalidnamechar(c) \
  ( (c == '-') || (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') )


int
parse_header(struct namevalue_t **nvs, char *header_str)
{
    int count = 0;

    enum {
        sw_name_start = 0,
        sw_name_end,
        sw_value_start,
        sw_value_end,
        sw_check_next_line,
        sw_field_done,
        sw_goto_next_line,
    } state = sw_name_start;

    char *n_a = NULL;   /*  name start       */
    char *n_z = NULL;   /*  name end         */
    char *v_a = NULL;   /*  value start      */
    char *v_z = NULL;   /*  value end        */
    int   n_l;          /*  length of name   */
    int   v_l;          /*  length of value  */

    char *p = header_str;
    int loop = 1;

    while (loop) {
        switch (state) {
            /* name start */
        case sw_name_start:
            if (*p == LF || *p == CR) {
                loop = 0;
                break;
            }
            if (isvalidnamechar(*p)) {
                n_a = p;
                state = sw_name_end;
            } else {
                state = sw_goto_next_line;
            }
            break;

            /* name end */
        case sw_name_end:
            while (isvalidnamechar(*p)) {
                p++;
            }
            n_z = p;
            /* space after name*/
            while (*p == SP || *p == HT) {
                p++;
            }
            if (*p == ':') {
                state = sw_value_start;
            } else {
                state = sw_goto_next_line;
            }
            p++;
            break;

            /* value start */
        case sw_value_start:
            /* space before value */
            while (*p == SP || *p == HT) {
                p++;
            }
            v_a = p;
            state = sw_value_end;
            break;

            /* value end */
        case sw_value_end:
            while (*p != LF && *p) {
                p++;
            }
            v_z = p;
            /* space after value */
            do {
                v_z--;
            } while (*v_z == SP || *v_z == HT || *v_z == CR);
            v_z++;
            state = sw_check_next_line;
            break;

        case sw_check_next_line:
            state = sw_field_done;
            if (p[0] == LF) {
                if (p[1] == SP || p[1] == HT) {
                    p++;
                    /* multi lines value */
                    state = sw_value_end;
                }
            }
            break;

        case sw_field_done:
            n_l = n_z - n_a;
            v_l = v_z - v_a;
            if (n_l > 0 && v_l > 0) {
                namevalues_add(nvs, n_a, n_l, v_a, v_l);
                count++;
            }
            state = sw_goto_next_line;
            break;

        case sw_goto_next_line:
            while (*p != LF && *p) {
                p++;
            }
            if (p[0] == LF) {
                if (p[1]) {
                    p++;
                    state = sw_name_start;
                    break;
                }
            }
            loop = 0;
            break;

        default:
            loop = 0;
            break;
        }
    }
    return count;
}
