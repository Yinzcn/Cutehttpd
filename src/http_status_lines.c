
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "http_status_lines.h"


static char *http_status_lines_1xx[] = {
    "100 Continue",
    "101 Switching Protocols",
    "102 Processing"
};
#define http_status_lines_1xx_max 102


static char *http_status_lines_2xx[] = {
    "200 OK",
    "201 Created",
    "202 Accepted",
    "203 Non-Authoritative Information",
    "204 No Content",
    "205 Reset Content",
    "206 Partial Content",
    "207 Multi-Status"
};
#define http_status_lines_2xx_max 207


static char *http_status_lines_3xx[] = {
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "",  /* "306 unused", */
    "307 Temporary Redirect"
};
#define http_status_lines_3xx_max 307


static char *http_status_lines_4xx[] = {
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Time-out",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Large",
    "415 Unsupported Media Type",
    "416 Requested Range Not Satisfiable",
    "417 Expectation Failed",
    "",  /* "418 unused", */
    "",  /* "419 unused", */
    "",  /* "420 unused", */
    "",  /* "421 unused", */
    "422 Unprocessable Entity",
    "423 Locked",
    "424 Failed Dependency"
};
#define http_status_lines_4xx_max 424


static char *http_status_lines_5xx[] = {
    "500 Internal Server Error",
    "501 Method Not Implemented",
    "502 Bad Gateway",
    "503 Service Temporarily Unavailable",
    "504 Gateway Time-out",
    "505 HTTP Version Not Supported",
    "506 Variant Also Negotiates",
    "507 Insufficient Storage",
    "",  /* "508 unused", */
    "",  /* "509 unused", */
    "510 Not Extended"
};
#define http_status_lines_5xx_max 510


char *
http_status_lines_get(int code)
{
    static char *unknown = "500 Unknown Status Code";
    switch (code/100) {
    case 1: /* 1xx */
        if (code > http_status_lines_1xx_max) {
            return unknown;
        }
        return http_status_lines_1xx[code - 100];

    case 2: /* 2xx */
        if (code > http_status_lines_2xx_max) {
            return unknown;
        }
        return http_status_lines_2xx[code - 200];

    case 3: /* 3xx */
        if (code > http_status_lines_3xx_max) {
            return unknown;
        }
        return http_status_lines_3xx[code - 300];

    case 4: /* 4xx */
        if (code > http_status_lines_4xx_max) {
            return unknown;
        }
        return http_status_lines_4xx[code - 400];

    case 5: /* 5xx */
        if (code > http_status_lines_5xx_max) {
            return unknown;
        }
        return http_status_lines_5xx[code - 500];

    default:
        return unknown;
    }
}
