/*
 * BASE150000 v1.0 (C89)
 * BAZ IPv6 MAP6 Atom
 * "By the fog, the impossible dies"
 * Fixes: non-ASCII removed, ipstr init,
 * recv() loop, timeout, port validation,
 * POSIX flags for C89/obsolete Linux
 */
#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#define BAZ_TIMEOUT_SEC   5
#define BAZ_RECV_BUF_SIZE 4096
#define BAZ_PORT_MIN      1
#define BAZ_PORT_MAX      65535
static int baz_validate_port(const char *port) {
    char *endptr; long val;
    if (!port || !*port) return -1;
    val = strtol(port, &endptr, 10);
    if (*endptr != '\0') return -1;
    if (val < BAZ_PORT_MIN || val > BAZ_PORT_MAX) return -1;
    return 0;
}
static int baz_set_timeout(int sockfd) {
    struct timeval tv;
    tv.tv_sec = BAZ_TIMEOUT_SEC; tv.tv_usec = 0;
    if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof tv)<0) return -1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&tv,sizeof tv)<0) return -1;
    return 0;
}
int baz_ipv6_connect(const char *hostname, const char *port) {
    struct addrinfo hints, *res, *p;
    int sockfd = -1, status;
    char ipstr[INET6_ADDRSTRLEN] = {0};
    if (baz_validate_port(port) != 0) return -1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_V4MAPPED;
    if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        fprintf(stderr, "BAZ: FETCH failed: %s\n", gai_strerror(status));
        return -1;
    }
    for (p = res; p != NULL; p = p->ai_next) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        inet_ntop(p->ai_family, &(ipv6->sin6_addr), ipstr, sizeof ipstr);
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        baz_set_timeout(sockfd);
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd); sockfd = -1;
            memset(ipstr, 0, sizeof ipstr);
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    if (sockfd == -1) { fprintf(stderr, "BAZ: No IPv6 route.\n"); return -1; }
    printf("BAZ: SUCCESS (OK) | [%s]:%s\n", ipstr, port);
    return sockfd;
}
static void baz_recv_response(int sockfd) {
    char buf[BAZ_RECV_BUF_SIZE]; ssize_t n;
    while ((n = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = '\0'; printf("%s", buf);
    }
}
int main(void) {
    const char *msg = "GET / HTTP/1.0\r\nHost: ipv6.google.com\r\n\r\n";
    int sock;
    printf("BASE150000 v1.0 | By the fog, the impossible dies\n");
    sock = baz_ipv6_connect("ipv6.google.com", "80");
    if (sock != -1) {
        if (send(sock, msg, strlen(msg), 0) >= 0) baz_recv_response(sock);
        close(sock);
    }
    return 0;
}
