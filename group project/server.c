//
//  server.c
//  sws
//
//  Created by FuBin Mo on 2020/11/19.
//

#include "server.h"

#define LISTENSIZE 5

int
createIpv4Socket(u_short *port, const char *ip){
    int sock;
    socklen_t length;
    struct sockaddr_in server;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("opening ipv4 stream socket");
        exit(EXIT_FAILURE);
    }
    
    server.sin_family = AF_INET;
    // because there will encounter problems with the network order of bytes and the
    // order of hosts, so we use conversion function between network byte order and local
    // byte order
    server.sin_port = htons(*port);

    if (ip)
        server.sin_addr.s_addr = inet_addr(ip);
    else
        server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sock, (struct sockaddr *)&server, sizeof(server)) != 0){
        perror("binding ipv4 stream socket error");
        exit(EXIT_FAILURE);
    }
    
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
        perror("getting ipv6 socket name");
        exit(EXIT_FAILURE);
    }
    
    (void)printf("Ipv4 Socket has port #%d\n", ntohs(server.sin_port));
    (void)printf("Ipv4 Socket has ip #%s\n", inet_ntoa(server.sin_addr));
    
    if(listen(sock,LISTENSIZE) == -1){
        perror("ipv4 listen error");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int
createIpv6Socket(u_short *port, const char *ip){
    int sock;
    socklen_t length;
    struct sockaddr_in6 server;
    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
        perror("opening ipv6 stream socket");
        exit(EXIT_FAILURE);
    }
    
    server.sin6_family = AF_INET6;
    server.sin6_port = htons(*port);
    
    if(ip)
        inet_pton(AF_INET6, ip, &server.sin6_addr);
    else
        server.sin6_addr = in6addr_any;
    
    if((bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in6))) == -1 ){
        perror("binding ipv6 stream socket error");
        exit(EXIT_FAILURE);
    }
    
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *)&server, &length) != 0) {
        perror("getting ipv6 socket name");
        exit(EXIT_FAILURE);
    }
    
    if(listen(sock, LISTENSIZE) == -1){
        perror("ipv6 listen error");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int
is_valid_ipv4(const char *ipv4){
    struct in_addr addr;
    if(ipv4 == NULL)
        return 0;
    if(inet_pton(AF_INET, ipv4, (void *)&addr) == 1)
        return 1;
    return 0;
}

int
is_valid_ipv6(const char *ipv6){
    struct in6_addr addr6;
    if(ipv6 == NULL)
        return 0;
    if(inet_pton(AF_INET6, ipv6, (void *)&addr6) == 1)
        return 1;
    return 0;
}

void *
handle_request(void* client) {
    return client;
}

