//
//  server.h
//  sws
//
//  Created by FuBin Mo on 2020/11/19.
//

#ifndef server_h
#define server_h

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

#endif /* server_h */

int createIpv4Socket(u_short *port, const char *ip);
int createIpv6Socket(u_short *port, const char *ip);

int is_valid_ipv4(const char *ipv4);
int is_valid_ipv6(const char *ipv6);

void * handle_request(void* client);
