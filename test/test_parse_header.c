
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>


#include "../src/base.c"
#include "../src/namevalue.c"
#include "../src/parse_header.c"


int
main()
{
    struct namevalue_t *nvs = NULL;
    char *hstr = file_get("http_header.txt");
    parse_header(&nvs, hstr);
    namevalues_each(&nvs, printf, "[%s]=>[%s]\n");
    namevalues_destroy(&nvs);
    free(hstr);
    return 0;
}
