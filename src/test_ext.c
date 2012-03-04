
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "chtd.h"
#include "test_ext.h"


int
test_ext(struct reqs_t *http_reqs)
{
    char *body = calloc(20480, sizeof(char));

    snprintf(body, 20480, "<pre>\r\n"
             "Time: [%s]\r\n"
             "method: [%s]\r\n"
             "uri: [%s]\r\n"
             "http_version: [%s]\r\n"
             "request_path: [%s]\r\n"
             "real_path: [%s]\r\n"
             "query_string: [%s]\r\n"
             "server_addr: [%s:%s]\r\n"
             "client_addr: [%s:%s]\r\n"
             "</pre>",
             x_nowstr(),
             http_reqs->method_name,
             http_reqs->uri,
             http_reqs->http_version_name,
             http_reqs->request_path,
             http_reqs->real_path,
             http_reqs->query_string,
             http_reqs->conn->server_addr,
             http_reqs->conn->server_port,
             http_reqs->conn->client_addr,
             http_reqs->conn->client_port);

    set_http_status  (http_reqs, 200);
    set_http_header  (http_reqs, "Content-Type", "text/html");
    set_http_header_x(http_reqs, "Content-Length", "%d", strlen(body));
    send_http_header (http_reqs);

    reqs_conn_send   (http_reqs, body, strlen(body));
    return 1;
}
