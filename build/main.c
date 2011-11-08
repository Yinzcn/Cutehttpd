
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


#include "../src/cutehttpd.c"

#include "assign_mime_types.c"


void *
uhook_time(void *reqs)
{
    reqs_throw_status(reqs, 200, x_nowstr());
    return (void *)1;
}


void *
uhook_test_2(void *data)
{
    printf("%s(\"0x%08x\")\n", __FUNCTION__, (int)data);
    return (void *)2;
}


/*
uhook Ŀ�꺯��, ��ȷ������ 0, ��������ֵ��ͻ������ Bad Request
*/
int
server_status(void *reqs)
{
    /* response body */
    char *wkinfo;
    char *body = malloc(20480);
    memset(body, 0, 20480);

    wkinfo = chtd_get_status_info(reqs_get_htdx(reqs), "html");
    snprintf(body, 20480, "<html><head><title>Cutehttpd Server Status Hook</title></head><body><br />Your wker ID: %d<br />\r\n%s</body></html>", wker_get_id(reqs_get_wker(reqs)), wkinfo);
    /*free(wkinfo);*/

    set_http_status  (reqs, 200); /* "200 OK" */
    set_http_header  (reqs, "Content-Type", "text/html");
    set_http_header_x(reqs, "Content-Length", "%d", strlen(body));
    send_http_header (reqs); /* ���� http ͷm*/

    /* ���� http ���� */
    reqs_conn_send   (reqs, body, strlen(body));
    free(body);

    /*
    ��ȷ������, �������
    */
    return 1;
}


int
main(int argc, char *argv[])
{
    /*
        ����һ������������ chtd
        ���� void * ���� int ���棬�������Ի��������һ����������
    */
    char ch;
    void *chtd;
    chtd = chtd_create();

    /*
        ��������
    */
    chtd_set_opt(chtd, "addr", "0.0.0.0");      /* http �󶨷����ַ */
    chtd_set_opt(chtd, "port", "8080");         /* http �󶨷���˿� */
    chtd_set_opt(chtd, "max_workers", "100");   /* ��� wkers ����, ������󲢷����� */
    chtd_set_opt(chtd, "keep_alive", "15");     /* ���� keep_alive ��ʱ, 0 ���� */
    chtd_set_opt(chtd, "max_post_size", "128"); /* ��� POST ����, ��λ MB */


    /*
        ���� fastcgi�������� [����������]��chtdx ���Զ����� fcgi ���̡�
        chtd_set_fcgi (chtd, ".��չ", "��ַ", "�˿�", "[����������]");
    */
    #ifdef CHTD_FCGI
    chtd_set_fcgi(chtd, ".php", "127.0.0.1", "9900", "php\\php-cgi.exe -b @addr@:@port@");
    #endif


    /*
        ��������
        chtd_set_vhost(chtd, "������", "��Ŀ¼", "����");
    */
    chtd_set_vhost(chtd, "127.0.0.11", "./vhosts/vhost1", "1");
    chtd_set_vhost(chtd, "127.0.0.12", "./vhosts/vhost2", "1");
    chtd_set_vhost(chtd, "127.0.0.13", "./vhosts/vhost3", "1");
    chtd_set_vhost(chtd, "*", "./htdocs", "1");  /* Ĭ������ */


    /*
        URI ���ӣ�Ƕ��ʽ����Ҫʵ��
        chtd_set_uhook(chtd, "host", "uri", (*)func);
        ���һ������ƥ�� (host��uri) һ������, ������ func �ĺ���
        ע��: uri ʹ��������ʽ, ��������ֱ������ƥ�䡣
        host ���� * ƥ������������
    */
    chtd_set_uhook(chtd, "*", "/uhook/time", uhook_time);
    chtd_set_uhook(chtd, "*", "/server-status", server_status);
    /*
        "*" ƥ����������
    */
    chtd_set_uhook(chtd, "*", "/uhook2", uhook_test_2);

    assign_mime_types(chtd);

    /*
        chtd_start() ���� Cutehttpd ����
        Cutehttpd �������ڲ������߳�����
    */
    if (chtd_start(chtd) == 0) { /* �ɹ����� 0��ʧ�ܷ��� -1 */
        chtd_log(chtd, "Cutehttpd �����ɹ�!");
    } else {
        chtd_log(chtd, "Cutehttpd ����ʧ��!");
    }

    while (chtd_get_status(chtd) != CHTD_STOPPED) {
        ch = getchar();
        switch (ch) {
        case 's':
            chtd_stop(chtd);
            break;
        case 'p':
            break;

        case 0xa:
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
