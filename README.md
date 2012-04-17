Cutehttpd 嵌入式 HTTP 服务器
============================

Cutehttpd 是用纯 C 写的一个小巧的嵌入式 http 服务器。Cutehttpd 尽可能将协议分化为对象类，这使得上一层的开发变得更容易，上层调用只需考虑逻辑处理。无请求状态下约占用 2MB 内存，1000 个请求约占用 4MB 内存。可执行文件体积约为 70KB。


特性
----
支持 FastCGI；URI 钩子；目录列表；MIME Type；


计划
----
支持 SSL；动态加载符合标准 JSON 的配置文件。


应用
----
远程控制面板，以及其他基于 Web 的产品开发。


协议
----
使用 GNU Lesser General Public License v3.0 协议发布。
简而言之，链接到 Cutehttpd 库的产品可以不公开源代码，但是对 Cutehttpd 源码进行修改必须公开源代码，并使用 LGPL 协议发布。


状态
----
基本成形，可以尝试进行二次开发。
欢迎提出建议 YinzCN_@_gmail.com。


示例
----
Cutehttpd 将支持多种编程语言，包括 C/C++, Ruby, Lua, C#

下面用 C 语言演示了将如何使用 Cutehttpd：


    #include "../src/cutehttpd.c"

    int
    main(int argc, char *argv[])
    {
      /*
        创建一个服务器对象 htdx
        可用 void * 或者 int 保存，其他语言或许可以用一个整数类型。
        可以同时创建多个 htdx，绑定到不同的端口进行服务。
      */
      void *htdx;
      htdx = htd_create();

      /*
        基本配置
      */
      htd_set_opt  (htdx, "addr", "127.0.0.1");   /* http 绑定服务地址 */
      htd_set_opt  (htdx, "port", "8080");        /* http 绑定服务端口 */
      htd_set_opt  (htdx, "max_workers", "1000"); /* 最大工作线程数, 决定最大并发请求 */
      htd_set_opt  (htdx, "keep_alive", "15");    /* 设置 keep_alive 超时, 0 禁用; */
      htd_set_opt  (htdx, "max_post_size", "8M"); /* 最大 POST 数据; */

      /*
        设置 fastcgi
        htd_set_fcgi (htdx, ".扩展", "地址", "端口", "[启动命令行]");
        若存在 [启动命令行]，htdx 将自动管理 fcgi 进程。
      */
      htd_set_fcgi (htdx, ".php", "127.0.0.1", "9000", "php\\php-cgi.exe -b @addr@:@port@");

      /*
        虚拟主机
        htd_set_vhost(htdx, "主机名", "根目录");
      */
      htd_set_vhost(htdx, "127.0.0.10", "./vhosts/host1", "1");
      htd_set_vhost(htdx, "127.0.0.20", "./vhosts/host2", "1");
      htd_set_vhost(htdx, "127.0.0.30", "./vhosts/host3", "1");
      /*
        默认虚拟主机
      */
      htd_set_vhost(htdx, "*", "htdocs", "1");

      /*
        URI 钩子，嵌入式的主要实现
        htd_set_xhook(htdx, "HostName", "URI", *FuncAddr);
        如果一个请求(Host与URI)匹配一个钩子, 将调用FuncAddr的函数
        注意: URI 使用正则表达式, 而主机名直接完整匹配
      */
      htd_set_xhook(htdx, "127.0.0.1", "/any/.*", xhook_test_1);
      htd_set_xhook(htdx, "127.0.0.1", "/server-status", server_status);
      htd_set_xhook(htdx, "127.0.0.1", "/xhook2-2", xhook_test_2);
      /*
        匹配任意主机
      */
      htd_set_xhook(htdx, "*", "/xhook2", xhook_test_2);

      /*
        启动 cutehttpd 服务
        cutehttpd 将会在内部创建线程运行
      */
      int Retn;
      Retn = htd_start(htdx);
      if (Retn) /* 成功返回 1 */
      {
        printf("cutehttpd 启动成功。");
      } else {
        printf("cutehttpd 启动失败！");
      }

      while (1) {
        /*
          [你的程序代码]
        */
        Sleep(20);
      }

      /*
        停止 ctuehttpd
      */
      htd_stop(htdx);

    }
