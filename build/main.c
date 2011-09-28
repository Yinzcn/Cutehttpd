
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>


#include "../src/_onefile_.c"

#include "assign_mime_types.c"


void *
uhook_test_1(void *data)
{
    printf("%s(\"%s\")\n", __FUNCTION__, (char *)data);
    return (void *)1;
}


void *
uhook_test_2(void *data)
{
    printf("%s(\"%s\")\n", __FUNCTION__, (char *)data);
    return (void *)2;
}


/*
uhook 目标函数, 正确处理返回 0, 返回其他值向客户端输出 Bad Request
*/
int
server_status(void *reqs)
{
    /* response body */
    char *body = malloc(20480);
    memset(body, 0, 20480);

    char *wkinfo;
    wkinfo = chtd_get_status_info(reqs_get_htdx(reqs), "html");
    snprintf(body, 20480, "<html><head><title>Cutehttpd Server Status Hook</title></head><body><br />Your wker ID: %d<br />\r\n%s</body></html>", wker_get_id(reqs_get_wker(reqs)), wkinfo);
    /*free(wkinfo);*/

    set_http_status  (reqs, 200); /* "200 OK" */
    set_http_header  (reqs, "Content-Type", "text/html");
    set_http_header_x(reqs, "Content-Length", "%d", strlen(body));
    send_http_header (reqs); /* 发送 http 头m*/

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
    void *chtd;
    chtd = chtd_create();

    /*
      基本配置
    */
    chtd_set_opt    (chtd, "addr", "0.0.0.0");      /* http 绑定服务地址 */
    chtd_set_opt    (chtd, "port", "8080");         /* http 绑定服务端口 */
    chtd_set_opt    (chtd, "max_workers", "1000");    /* 最大 wkers 数量, 决定最大并发数量 */
    chtd_set_opt    (chtd, "keep_alive", "15");     /* 设置 keep_alive 超时, 0 禁用 */
    chtd_set_opt    (chtd, "max_post_size", "128"); /* 最大 POST 数据, 单位 MB */


    /*
      设置 fastcgi。若存在 [启动命令行]，chtdx 将自动管理 fcgi 进程。
      chtd_set_fcgi (chtd, ".扩展", "地址", "端口", "[启动命令行]");
    */
    chtd_set_fcgi(chtd, ".php", "127.0.0.1", "9900", "php\\php-cgi.exe -b @addr@:@port@");


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
      chtd_set_uhook(chtd, "HostName", "URI", *FuncAddr);
      如果一个请求(Host与URI)匹配一个钩子, 将调用FuncAddr的函数
      注意: URI 使用正则表达式, 而主机名直接完整匹配
    */
    chtd_set_uhook(chtd, "127.0.0.1", "/any/.*",   uhook_test_1);
    chtd_set_uhook(chtd, "127.0.0.1", "/uhook2-2", uhook_test_2);
    chtd_set_uhook(chtd, "*", "/server-status",   server_status);
    /* "*" 匹配任意主机 */
    chtd_set_uhook(chtd, "*", "/uhook2", uhook_test_2);

    assign_mime_types(chtd);

    /*
    chtd_start() 启动 Cutehttpd 服务
    Cutehttpd 将会在内部创建线程运行
    */
    if (chtd_start(chtd) == 0) /* 成功返回 0，失败返回 -1 */
    {
        chtd_log(chtd, "Cutehttpd 启动成功!");
    }
    else
    {
        chtd_log(chtd, "Cutehttpd 启动失败!");
    }

    char ch;
    int loop = 1;
    while (chtd_get_status(chtd) != CHTD_STOPPED)
    {
        ch = getchar();
        switch (ch)
        {
        case 'e':
            chtd_stop(chtd);
            chtd_delete(chtd);
            loop = 0;
            break;

        case 'p':
            break;

        case 0xa:
        case 's':
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
    return 0;
}
