//
//  server.c
//  sws
//
//  Created by FuBin Mo on 2020/11/19.
//

#include "server.h"

#define LISTENSIZE 5

int
build_ipv4_socket(u_short *port, const char *ip){  //-p  -i
    int sockfd;
    struct sockaddr_in server_address;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("ipv4 socket error");
        exit(EXIT_FAILURE);
    }
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
    }
    memset(&server_address,0,sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(*port);
    if (ip)
        server_address.sin_addr.s_addr = inet_addr(ip);
    else
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1){
        perror("ipv4 binding error");
        exit(EXIT_FAILURE);
    }
    
    (void)printf("Socket has port #%d\n", ntohs(server_address.sin_port));
    (void)printf("Socket has ip #%d\n", ntohs(server_address.sin_addr.s_addr));
    
    if(listen(sockfd,LISTENSIZE) == -1){
        perror("ipv4 listen error");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int
build_ipv6_socket(u_short *port, const char *ip){
    int sockfd;
    struct sockaddr_in6 server_address;
    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
        perror("ipv6 socket error");
        exit(EXIT_FAILURE);
    }
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopet");
    }
    bzero(&server_address,sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_port = htons(*port);
    if(ip)
        inet_pton(AF_INET6, ip, &server_address.sin6_addr);
    else
        server_address.sin6_addr = in6addr_any;
    if((bind(sockfd, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in6))) == -1 ){
        perror("ipv6 binding error");
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd, LISTENSIZE) == -1){
        perror("ipv4 listen error");
        exit(EXIT_FAILURE);
    }
    return sockfd;
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
    int clientfd = *(int *)client;
    int rval;
    
    socklen_t length = sizeof(clientfd);
    
    struct sockaddr_storage addr;
    
    if (getpeername(clientfd, (struct sockaddr*)&addr, &length) == -1) {
        printf("log error");
        return NULL;
    }
    
    char ip_address[INET_ADDRSTRLEN];
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    u_short port = ntohs(s->sin_port);
    
    (void)printf("%d \n", port);
    
    do {
        (void)printf("1 \n");
        char buf[BUFSIZ];
        bzero(buf, sizeof(buf));
        if ((rval = read(clientfd, buf, BUFSIZ)) < 0) {
            perror("reading stream message");
        }

        if (rval == 0) {
            (void)printf("Ending connection\n");
        } else {
            const char *rip;
            if ((rip = inet_ntop(AF_INET, &s->sin_addr,ip_address, (socklen_t)INET_ADDRSTRLEN)) == NULL){
                perror("inet_ntop");
                rip = "unknown";
            }
            else {
                (void)printf("Client (%s) sent: %s\n", rip, buf);
            }
            
        }
    } while (rval != 0);
    (void)close(clientfd);
    return NULL;
}

