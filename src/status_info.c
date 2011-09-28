
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


#include "cutehttpd.h"
#include "status_info.h"


char *
chtd_get_status_info(struct htdx_t *htdx, char *format)
{
    static char buff[20480];

    enum inf_fmt_t nfmt;
    if (strcasecmp(format, "html") == 0)
    {
        nfmt = FMT_HTML;
    }
    else
    {
        nfmt = FMT_TEXT;
    }

    char start_at[32] = "";
    char been_run[32] = "";
    time_t t_been_run;
    t_been_run = difftime(time(NULL), htdx->birthtime);
    int run_day, run_hour, run_min, run_sec;
    run_day  = (t_been_run / 86400);
    run_hour = (t_been_run % 86400) / 3600;
    run_min  = (t_been_run %  3600) /   60;
    run_sec  = (t_been_run %    60);
    strftime(start_at, sizeof(start_at) - 1, "%Y-%m-%d %H:%M:%S", localtime(&htdx->birthtime));
    snprintf(been_run, sizeof(been_run) - 1, "%d days %02d:%02d:%02d", run_day, run_hour, run_min, run_sec);

    if (nfmt == FMT_HTML)
    {
        sprintf(buff, "<br />\r\n"
                "<b>Server Status</b><br />\r\n"
                "Server started at: %s<br />\r\n"
                "Server up: %s<br />\r\n"
                "nConn: %d<br />\r\n"
                "nReqs: %d<br />\r\n"
                "nBadReqs: %d<br />\r\n"
                "nIdelWkers: %d<br />\r\n"
                "<br />\r\n"
                "<b>wkers status</b><br />\r\n"
                "\"I\" = Idel<br />\r\n"
                "\"B\" = Busy<br />\r\n"
                "\"<u>*</u>\" = Waiting for Connection. Thread keepalive.<br />\r\n"
                "\"K\" = HTTP Keep-Alive<br />\r\n"
                "\"H\" = Hung<br />"
                "\"<u> </u>\" = Thread keepalive.<br />\r\n"
                "<pre>", start_at, been_run, htdx->nConn, htdx->nReqs, htdx->nBadReqs, htdx->nIdelWkers);
    }
    else
    {
        sprintf(buff, "  [Server Status] \n"
                "  Server started at: %s\n"
                "  Server has been run: %s\n"
                "  nConn: %d\n"
                "  nReqs: %d\n"
                "  nBadReqs: %d\n"
                "  nIdelWkers: %d\n"
                "  nWaitWkers: %d\n"
                "\n"
                "  [wkers Status]\n"
                "  \"I\" = Idel\n"
                "  \"*\" = Waiting for Connection. Thread keepalive.\n"
                "  \"B\" = Busy\n"
                "  \"K\" = HTTP Keep-Alive\n"
                "  \"H\" = Hung\n"
                "  \"_\" = Thread keepalive.\n"
                "\n"
                "  wkers [", start_at, been_run, htdx->nConn, htdx->nReqs, htdx->nBadReqs, htdx->nIdelWkers, htdx->nWaitWkers);
    }
    char status[] = "I*BKHU123456789abcdef";
    int n = strlen(buff);
    int i = 0;

    struct wker_t *curr, *last;
    curr = htdx->wkers;
    last = curr->prev;
    while (1)
    {
        if (i % 50 == 0)
        {
            if (nfmt == FMT_HTML)
            {
                n += sprintf(buff + n, "\r\n");
            }
            else
            {
                n += sprintf(buff + n, "\n  ");
            }
        }
        n += sprintf(buff + n, "%c%c", status[curr->status], curr->step);

        if (curr == last)
            break;
        curr = curr->next;
        i++;
    }

    if (nfmt == FMT_HTML)
    {
        n += sprintf(buff + n, "</pre>");
    }
    else
    {
        n += sprintf(buff + n, "\n  ]\n");
    }
    return buff;
}
