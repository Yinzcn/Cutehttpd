
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "http_send_file.h"


int
http_send_file(struct reqs_t *http_reqs, char *file_path)
{
    int maxtry = 1000;
    FILE *pFile;
    while (maxtry--)
    {
        pFile = fopen(file_path, "rb");
        if (pFile)
        {
            break;
        }
        Sleep(1);
    }
    if(!pFile)
    {
        chtd_cry(http_reqs->htdx, "ERR(%d):[%s]", (int)GetLastError(), file_path);
        reqs_throw_status(http_reqs, 404, "http_send_file() -> fopen() failed!");
        return 1;
    }

    /*
    [ get file size
    */
    int file_size;
    fseek(pFile, 0, SEEK_END);
    file_size = ftell(pFile);
    rewind(pFile);
    /*
    ]
    */

    set_http_status  (http_reqs, 200); /* "200 OK" */
    set_http_header  (http_reqs, "Date", "");
    set_http_header  (http_reqs, "Last-Modified", "");
    set_http_header_x(http_reqs, "Content-Type", get_mime_type(http_reqs->htdx, get_file_extname(file_path)));
    set_http_header_x(http_reqs, "Content-Length", "%d", file_size);
    send_http_header (http_reqs);

    int  n;
    char buff[8192];
    while ((n = fread(buff, 1, 8192, pFile)))
    {
        if (!reqs_conn_send(http_reqs, buff, n))
        {
            break;
        }
    }
    fclose(pFile);

    return 1;
}
