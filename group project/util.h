//
//  util.h
//  sws
//
//  Created by FuBin Mo on 2020/12/6.
//

#ifndef util_h
#define util_h

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#endif /* util_h */

int is_valid_ipv4(const char *ipv4);
int is_valid_ipv6(const char *ipv6);

int validate_path(const char *path);
int validate_port(int str);

void write_log(int clientfd, int log_fd);

char *replace (const char *url, const char *old_user, const char *new_user);
