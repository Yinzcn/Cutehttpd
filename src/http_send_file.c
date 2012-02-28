
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "http_send_file.h"


int
http_send_file(struct reqs_t *http_reqs, char *file_path)
{
    int maxtry = 100;
    int file_size;
    FILE *pFile;
    int  n;
    char buff[8192];
    while (maxtry--) {
        pFile = fopen(file_path, "rb");
        if (pFile) {
            break;
        }
        x_msleep(5);
    }
    if(!pFile) {
        chtd_cry(http_reqs->htdx, "http_send_file() -> fopen() failed! [%s]", file_path);
        reqs_throw_status(http_reqs, 404, "http_send_file() -> fopen() failed!");
        return 1;
    }

    /*
    [ get file size
    */
    fseek(pFile, 0, SEEK_END);
    file_size = ftell(pFile);
    rewind(pFile);
    /*
    ]
    */

    set_http_status  (http_reqs, 200); /* "200 OK" */
    set_http_header  (http_reqs, "Date", "");
    set_http_header  (http_reqs, "Last-Modified", "");
    set_http_header_x(http_reqs, "Content-Type", get_mime_type(http_reqs->htdx, x_ext_name(file_path)));
    set_http_header_x(http_reqs, "Content-Length", "%d", file_size);
    send_http_header (http_reqs);

    while (1) {
        if (!(n = fread(buff, 1, 8192, pFile))) {
            break;
        }
        if (!reqs_conn_send(http_reqs, buff, n)) {
            break;
        }
    }
    fclose(pFile);

    return 1;
}
