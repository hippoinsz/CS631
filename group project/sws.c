//
//  main.c
//  sws
//
//  Created by FuBin Mo on 2020/11/19.
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>

#include "server.h"

#define DEFAULTPORT 0

void
write_log(int clientfd, int log_fd);

int
main(int argc, char **argv) {
    
    int c_flag = 0,d_flag = 0,h_flag = 0,i_flag = 0,l_flag = 0;

    u_short port = DEFAULTPORT;

    int log_fd = 0;

    const char *cgi_path, *sws_dir, *ip, *log_file = NULL;
    
    int listenedfd = 0;
    int clientfd;
    struct sockaddr_in client;
    char opt;
    
    while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
        switch(opt){
            case 'c':
                c_flag = 1;
                cgi_path = optarg;
                break;
            case 'd':
                d_flag = 1;
                break;
            case 'h':
                h_flag = 1;
                break;
            case 'i':
                i_flag = 1;
                ip = optarg;
                break;
            case 'l':
                l_flag = 1;
                log_file = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: sws[−dh] [−c dir] [−i address] [−l file] [−p port] dir\n");
                exit(EXIT_FAILURE);

        }
    }
    
    sws_dir = argv[optind];
    if (sws_dir == NULL) {
        perror("NO DIR \n the command should be :sws[−dh] [−c dir] [−i address] [−l file] [−p port] dir\n\n");
        exit(EXIT_FAILURE);
    }
    
    if (l_flag && log_file != NULL) {
        if ((log_fd = open(log_file, O_CREAT| O_APPEND| O_RDWR, 0777)) <= 0 ){
            perror("create log_file error");
            exit(EXIT_FAILURE);
        }
    }
    
    if (c_flag)
        printf("%s", cgi_path);
        
    if (h_flag)
        printf("do something h");
    
    if (d_flag)
        printf("do something d");
    
    if (i_flag) {
        if (is_valid_ipv4(ip))
            listenedfd = createIpv4Socket(&port, ip);
        if (is_valid_ipv6(ip))
            listenedfd = createIpv6Socket(&port, ip);
    } else {
        listenedfd = createIpv4Socket(&port, ip);
    }
    
    int client_len = sizeof(client);
    
    while (1) {
        if((clientfd = accept(listenedfd, (struct sockaddr *)&client, (socklen_t *)&client_len)) == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        
        if (l_flag) {
            write_log(clientfd, log_fd);
        }

        handle_request(&clientfd);
        
        (void)close(listenedfd);
    }
    
    return  EXIT_SUCCESS;
}

void
write_log(int clientfd, int log_fd){
    socklen_t length;
    struct sockaddr_storage server;
    char buf[BUFSIZ];
    int size;
    
    time_t timep;
    struct tm *p;
    time(&timep);
    p = gmtime(&timep);
    
    if (getsockname(clientfd, (struct sockaddr *)&server, &length) != 0) {
        perror("write log getting socket name error");
        exit(EXIT_FAILURE);
    }
    
    if (server.ss_family == AF_INET) {
        char ip_address[INET_ADDRSTRLEN];
        struct sockaddr_in *s = (struct sockaddr_in *)&server;
        if (inet_ntop(AF_INET, &s->sin_addr,ip_address, (socklen_t)INET_ADDRSTRLEN) == NULL) {
            perror("write log ipv4 inet_ntop error");
        }
        
        sprintf(buf, "%s %d-%d-%dT%d:%d:%d",ip_address, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        
        size = (int)strlen(buf);
        
        if (write(log_fd, buf, size) != size){
            perror("write log ipv4 write file error");
        }
    }else {
        char ip_address[INET6_ADDRSTRLEN];
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&server;
        if (inet_ntop(AF_INET6, &s->sin6_addr, ip_address, INET6_ADDRSTRLEN) == NULL){
            perror("write log ipv6 inet_ntop error");
        }
        
        sprintf(buf, "%s %d-%d-%dT%d:%d:%d",ip_address, (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        
        size = (int)strlen(buf);
        
        if (write(log_fd, buf, size)!= size){
            perror("write log ipv6 write file error");
        }
    }
}
