
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "http_send_file.h"


int
http_send_file(struct reqs_t *http_reqs, char *file_path)
{
    FILE *pFile;
    int n;
    struct stat st = { 0 };
    int maxtry = 100;
    time_t rawtime;
    char lmd_time[32];
    char snd_time[32];
    char buff[8192];
    while (maxtry--) {
        pFile = fopen(file_path, "rb");
        if (pFile) {
            break;
        }
        x_msleep(5);
    }
    if (!pFile) {
        chtd_cry(http_reqs->htdx, "http_send_file() -> fopen() failed! [%s]", file_path);
        reqs_throw_status(http_reqs, 503, "http_send_file() -> fopen() failed!");
        return 1;
    }
    fstat(fileno(pFile), &st);
    rawtime = time(NULL);
    strftime(snd_time, sizeof(snd_time), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&rawtime));
    strftime(lmd_time, sizeof(lmd_time), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&st.st_mtime));
    set_http_status  (http_reqs, 200); /* "200 OK" */
    set_http_header  (http_reqs, "Date", snd_time);
    set_http_header  (http_reqs, "Last-Modified", lmd_time);
    set_http_header  (http_reqs, "Content-Type", get_mime_type(http_reqs->htdx, x_ext_name(file_path)));
    set_http_header_x(http_reqs, "Content-Length", "%d", st.st_size);
    send_http_header (http_reqs);
    while (1) {
        if (!(n = fread(buff, 1, sizeof(buff), pFile))) {
            break;
        }
        if (!reqs_conn_send(http_reqs, buff, n)) {
            break;
        }
    }
    fclose(pFile);
    return 1;
}
