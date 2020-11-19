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
#include <pthread/pthread.h>

#include "server.h"

#define DEFAULTPORT 65345

int c_flag = 0,d_flag = 0,h_flag = 0,i_flag = 0,l_flag = 0;

u_short port = DEFAULTPORT;

const char *cgi_path, *sws, *ip, *log_file = NULL;

int
main(int argc, char **argv) {
    
    int listenedfd = 0,listenedfd_ipv6 = 0;
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
        }
    }
    
    if (i_flag == 1) {
        if (is_valid_ipv4(ip))
            listenedfd = build_ipv4_socket(&port, ip);
        if (is_valid_ipv6(ip))
            listenedfd = build_ipv6_socket(&port, ip);
    }else {
        listenedfd = build_ipv4_socket(&port, ip);
        listenedfd_ipv6 = build_ipv6_socket(&port, ip);
    }
    
    int client_len = sizeof(client);
    /* use a new thread to listen ipv6 */
    pthread_t client_thread;
//        if (pthread_create(&ipv6_thread, NULL, accept_ipv6, (void*)&listenedfd_ipv6) != 0)
//            perror("create ipv6 error");
    while (1) {
        if((clientfd = accept(listenedfd, (struct sockaddr *)&client, (socklen_t *)&client_len)) == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        
        if(d_flag)
            handle_request(&clientfd);
        else { /* create a thread to handle 2 request */
            if(pthread_create(&client_thread, NULL, handle_request, &clientfd) != 0)
                perror("create thread error");
        }
        
        (void)close(listenedfd);
    }
    
    return  EXIT_SUCCESS;
}
