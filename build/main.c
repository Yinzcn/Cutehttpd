
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


#include "../src/cutehttpd.c"

#include "assign_mime_types.c"


int
uhook_time(void *reqs)
{
    each_http_post(reqs, printf, "[%s] = [%s]\n");
    reqs_throw_status(reqs, 200, x_nowstr());
    return 1;
}


int
uhook_demo_1(void *reqs)
{
    reqs_throw_status(reqs, 200, "uhook_demo_1");
    return 1;
}


int
uhook_demo_2(void *reqs)
{
    reqs_cont_push(reqs, "a=");
    reqs_cont_push(reqs, get_http_post(reqs, "a"));
    reqs_cont_push(reqs, "\r\n");
    reqs_cont_push(reqs, "b=");
    reqs_cont_push(reqs, get_http_post(reqs, "b"));
    reqs_cont_push(reqs, "\r\n");
    reqs_cont_send(reqs);
    return 1;
}


int
server_status(void *reqs)
{
    /* response body */
    char *wkinfo;
    char *body = malloc(20480);
    memset(body, 0, 20480);

    wkinfo = chtd_get_status_info(reqs_get_htdx(reqs), "html");
    snprintf(body, 20480, "<html>"
        "<head><title>Cutehttpd Server Status Hook</title></head>"
        "<body><br />Your wker ID: %d<br />\r\n%s</body>"
        "</html>", wker_get_id(reqs_get_wker(reqs)), wkinfo);
    /*free(wkinfo);*/

    set_http_status  (reqs, 200); /* "200 OK" */
    set_http_header  (reqs, "Content-Type", "text/html");
    set_http_header_x(reqs, "Content-Length", "%d", strlen(body));
    send_http_header (reqs); /* 发送 http 头 */

    /* 发送 http 主体 */
    reqs_conn_send   (reqs, body, strlen(body));
    free(body);

    /*
    正确的请求, 处理完毕
    */
    return 1;
}


int
main(int argc, char *argv[])
{
    /*
        创建一个服务器对象 chtd
        可用 void * 或者 int 保存，其他语言或许可以用一个整数类型
    */
    char ch;
    void *chtd;
    chtd = chtd_create();

    /*
        基本配置
    */
    chtd_set_opt(chtd, "addr", "0.0.0.0");      /* http 绑定服务地址 */
    chtd_set_opt(chtd, "port", "8080");         /* http 绑定服务端口 */
    chtd_set_opt(chtd, "max_workers", "100");   /* 最大 wkers 数量, 决定最大并发数量 */
    chtd_set_opt(chtd, "keep_alive", "15");     /* 设置 keep_alive 超时, 0 禁用 */
    chtd_set_opt(chtd, "max_post_size", "128"); /* 最大 POST 数据, 单位 MB */


    /*
        设置 fastcgi。若存在 [启动命令行]，chtdx 将自动管理 fcgi 进程。
        chtd_set_fcgi (chtd, ".扩展", "地址", "端口", "[启动命令行]");
    */
    #ifdef CHTD_FCGI
    //chtd_set_fcgi(chtd, ".php", "127.0.0.1", "9000", "");
    chtd_set_fcgi(chtd, ".php", "127.0.0.1", "9000", "..\\..\\php\\php-cgi.exe -b @addr@:@port@");
    #endif


    /*
        虚拟主机
        chtd_set_vhost(chtd, "主机名", "根目录", "配置");
    */
    chtd_set_vhost(chtd, "127.0.0.11", "./vhosts/vhost1", "1");
    chtd_set_vhost(chtd, "127.0.0.12", "./vhosts/vhost2", "1");
    chtd_set_vhost(chtd, "127.0.0.13", "./vhosts/vhost3", "1");
    chtd_set_vhost(chtd, "*", "./htdocs", "1");  /* 默认主机 */


    /*
        URI 钩子，嵌入式的主要实现
        chtd_set_uhook(chtd, "host", "uri", (*)func);
        如果一个请求匹配 (host与uri) 一个钩子, 将调用 func 的函数
        注意: uri 使用正则表达式, 而主机名直接完整匹配。
        host 可用 * 匹配所有主机。
    */
    chtd_set_uhook(chtd, "*", "/uhook/time", uhook_time);
    chtd_set_uhook(chtd, "*", "/server-status", server_status);
    /*
        "*" 匹配任意主机
    */
    chtd_set_uhook(chtd, "*", "/uhook_demo_1", uhook_demo_1);
    chtd_set_uhook(chtd, "*", "/uhook_demo_2", uhook_demo_2);

    assign_mime_types(chtd);

    /*
        chtd_start() 启动 Cutehttpd 服务
        Cutehttpd 将会在内部创建线程运行
    */
    if (chtd_start(chtd) == 0) { /* 成功返回 0，失败返回 -1 */
        chtd_log(chtd, "Cutehttpd 启动成功!");
    } else {
        chtd_log(chtd, "Cutehttpd 启动失败!");
    }

    while (chtd_get_status(chtd) != CHTD_STOPPED) {
        ch = getchar();
        switch (ch) {
        case 's':
            chtd_stop(chtd);
            break;
        case 'p':
            break;

        case 0xA:
            printf("%s", chtd_get_status_info(chtd, "text"));
            break;

        case 'h':
#ifdef DEBUG
            chtd_print_uhooks(chtd);
#endif
            break;

        case 'v':
#ifdef DEBUG
            chtd_print_vhosts(chtd);
#endif
            break;

        default:
            break;
        }
    }
    printf("%d", chtd_delete(chtd));
    return 0;
}
