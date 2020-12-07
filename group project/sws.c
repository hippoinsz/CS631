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
#include "util.h"

#define DEFAULTPORT 8080

int c_flag = 0, d_flag = 0, h_flag = 0, l_flag = 0, p_flag = 0, i_flag = 0;

void
write_log(int clientfd, int log_fd);

int
main(int argc, char **argv) {

    u_short port = DEFAULTPORT;

    int log_fd = 0;

    char *cgi_path = NULL, *sws_dir = NULL, *ip = NULL, *log_file = NULL;
    
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
                p_flag = 1;
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
    
    if (h_flag == 1) {
        printf("Usage: sws[−dh] [−c dir] [−i address] [−l file] [−p port] dir\n");
        exit(EXIT_SUCCESS);
    }
    
    if (!validate_path(sws_dir)) {
        fprintf(stderr, "%s is not a validated directory!\n", sws_dir);
        exit(EXIT_FAILURE);
    }
    
    if (l_flag && log_file != NULL) {
        if ((log_fd = open(log_file, O_CREAT| O_APPEND| O_RDWR, 0777)) <= 0 ){
            perror("create log_file error");
            exit(EXIT_FAILURE);
        }
    }
    
    if (c_flag) {
        if (cgi_path != NULL && !validate_path(cgi_path)) {
            fprintf(stderr, "%s is not a validated CGI directory!\n", cgi_path);
            exit(EXIT_FAILURE);
        }
    }
        
    if (p_flag) {
        if (!validate_port(port)) {
            fprintf(stderr, "Invalid port number!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    if (d_flag)
        printf("Debuging mode\n");
    else {
        if (daemon(1, 0) == -1) {
            fprintf(stderr, "daemon err!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    if (i_flag && !is_valid_ipv4(ip) && !is_valid_ipv6(ip)){
        perror("no vaild ip to create server");
        exit(EXIT_FAILURE);
    }
    
    deal_network(&port, ip, sws_dir, log_fd);
    
    return  EXIT_SUCCESS;
}
