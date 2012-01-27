
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "http_list_dir.h"


int
http_list_dir(struct reqs_t *reqs, char *path)
{
    DIR *dirx;
    struct dirent *dPtr;
    char *req_path_ori = reqs->request_path;
    char *req_path_dec = calloc(strlen(req_path_ori) + 1, sizeof(char));

    url_decode(req_path_ori, req_path_dec);

    if (req_path_ori[strlen(req_path_ori) - 1] != '/') {
        reqs_cont_push_x(reqs, "<html>\r\n"
            "<head>\r\n"
            "<title>301 Moved Permanently</title>\r\n"
            "</head>\r\n"
            "<body>\r\n"
            "<h1>Moved Permanently</h1>\r\n"
            "<p>The document has moved <a href=\"%s/\">here</a>.</p>\r\n"
            "<hr />\r\n"
            "<address>%s</address>\r\n"
            "</body>\r\n"
            "</html>", req_path_ori, reqs->htdx->SERVER_SOFTWARE);
        set_http_status  (reqs, 301);
        set_http_header_x(reqs, "Location", "%s/", req_path_ori);
        reqs_cont_send   (reqs);
        free(req_path_dec);
        return 1;
    }

    /* [ */
    dirx = opendir(path);

    if (!dirx) {
        reqs_throw_status(reqs, 404, req_path_ori);
        free(req_path_dec);
        return 1;
    }

    reqs_cont_push_x(reqs,
        "<!DOCTYPE html>\r\n"
        "<html>\r\n"
        "<head>\r\n"
        "<title>Index of %s</title></head>\r\n"
        "<body>\r\n"
        "<h1>Index of %s</h1>\r\n"
        "<pre>\r\n"
        "<hr>\r\n", req_path_dec, req_path_dec);

    while ((dPtr = readdir(dirx))) {
        char href[1024];
        url_encode(dPtr->d_name, href, 1024 - 1);
        reqs_cont_push_x(reqs, "<a href=\"%s\">%s</a>\r\n", href, dPtr->d_name);
    }

    reqs_cont_push_x(reqs,
        "</pre>\r\n"
        "<hr>\r\n"
        "<strong>%s</strong>\r\n"
        "</body>\r\n"
        "</html>", reqs->htdx->SERVER_SOFTWARE);

    closedir(dirx);
    /* ] */

    set_http_status(reqs, 200); /* "200 OK" */
    reqs_cont_send (reqs);

    free(req_path_dec);
    return 1;
}
