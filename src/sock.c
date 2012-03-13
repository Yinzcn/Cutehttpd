
/**
  @author Ying Zeng (YinzCN_at_Gmail.com)
**/


void
sock_non_blocking(struct sock_t *sock)
{
#ifdef _WIN32
    unsigned long u = 1;
    ioctlsocket(sock->socket, FIONBIO, &u);
#else
    (void)fcntl(sock->socket, F_SETFL, fcntl(sock->socket, F_GETFL, 0) | O_NONBLOCK);
#endif
}


void
set_sock_timeout(struct sock_t *sock, int optn, int msec)
{
#ifdef _WIN32
    setsockopt(sock->socket, SOL_SOCKET, optn, (void *)&msec, sizeof(msec));
#else
    struct timeval tval;
    tval.tv_sec  = 0;
    tval.tv_usec = msec * 1000;
    setsockopt(sock->socket, SOL_SOCKET, optn, (void *)&tval, sizeof(tval));
#endif
}


void
sock_close(struct sock_t *sock)
{
    if (sock == NULL) {
        return;
    }
    if (sock->socket > 0) {
        static char buff[256];
        int o = 1;
        struct linger l;
        l.l_onoff  = 1;
        l.l_linger = 0;
        //setsockopt(sock->socket, SOL_SOCKET, SO_LINGER,     (void *)&l, sizeof(l));
        setsockopt(sock->socket, SOL_SOCKET, SO_DONTLINGER, (void *)&o, sizeof(o));
        shutdown(sock->socket, SHUT_WR);
        sock_non_blocking(sock);
        while (recv(sock->socket, buff, 256, 0) > 0);
        closesocket(sock->socket);
        sock->socket = 0;
    }
}
