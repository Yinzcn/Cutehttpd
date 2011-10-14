
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "http_list_dir.h"


int
http_list_dir(struct reqs_t *http_reqs, char *path)
{
    /*
    struct dirent
    {
      long           d_ino;                // Always zero.
      unsigned short d_reclen;             // Always zero.
      unsigned short d_namlen;             // Length of name in d_name.
      char           d_name[FILENAME_MAX]; // File name.
    };
    */

    DIR *dirx;

    int buffsize;
    int datasize;
    char *buff;
    struct dirent *dPtr;

    char *request_path = http_reqs->request_path;
    char *request_path_decoded = calloc(strlen(request_path) + 1, sizeof(char));
    url_decode(request_path, request_path_decoded);
    if (request_path[strlen(request_path) - 1] != '/') {
        set_http_status  (http_reqs, 301);
        set_http_header  (http_reqs, "Content-Length", "0");
        set_http_header_x(http_reqs, "Location", "%s/", request_path);
        send_http_header (http_reqs);
        free(request_path_decoded);
        return 1;
    }

    /* [ */
    dirx = opendir(path);

    if (!dirx) {
        reqs_throw_status(http_reqs, 404, request_path);
        free(request_path_decoded);
        return 1;
    }

    buffsize = 8192;
    datasize = 0;
    buff = calloc(buffsize, sizeof(char));

    datasize += sprintf(buff + datasize,
                        "<!DOCTYPE html>\r\n"
                        "<html>\r\n"
                        "<head>\r\n"
                        "<title>Index of %s</title></head>\r\n"
                        "<body>\r\n"
                        "<h1>Index of %s</h1>\r\n"
                        "<pre>\r\n"
                        "<hr>\r\n", request_path_decoded, request_path_decoded);

    while ((dPtr = readdir(dirx))) {
        char Href[1024];
        url_encode(dPtr->d_name, Href, 1024 - 1);
        datasize += sprintf(buff + datasize, "<a href=\"%s\">%s</a>\r\n", Href, dPtr->d_name);
        if (buffsize - datasize < 1024) {
            buffsize += 8192;
            buff = realloc(buff, buffsize);
        }
    }

    datasize += sprintf(buff + datasize,
                        "</pre>\r\n"
                        "<hr>\r\n"
                        "<strong>%s</strong>\r\n"
                        "</body>\r\n"
                        "</html>", http_reqs->htdx->SERVER_SOFTWARE);

    closedir(dirx);
    /* ] */

    set_http_status   (http_reqs, 200);
    set_http_header   (http_reqs, "Content-Type", "text/html");
    set_http_header_x (http_reqs, "Content-Length", "%d", datasize);
    send_http_header  (http_reqs);
    reqs_conn_send    (http_reqs, buff, datasize);

    free(buff);
    free(request_path_decoded);
    return 1;
}
