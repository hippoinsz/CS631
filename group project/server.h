//
//  server.h
//  sws
//
//  Created by FuBin Mo on 2020/11/19.
//

#ifndef server_h
#define server_h

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <libgen.h>
#include <magic.h>

#include "util.h"
#include "cgi.h"

#endif /* server_h */

extern int d_flag, i_flag, l_flag, c_flag;

void * accept_ipv6(void* listened);

int createIpv4Socket(u_short *port, const char *ip);
int createIpv6Socket(u_short *port, const char *ip);

void deal_network(u_short *port, const char *ip, char *sws_dir, int log_fd,
                  const char *cgi_dir
                  );

int get_request_info(int socket, char *buf, int size);

void * handle_request(void* client);

void handle_head(int clientfd, const char *path);
void handle_get(int clientfd, const char *path, const char *modify);

int formate_date(int clientfd);

void handle_server(char *ip, char *port);

int send_timestamp(int clientfd, const char *path);

void send_content(int clientfd, const char *path);

void send_error(int clientfd, char *str, const char *path);

void send_success(int clientfd, const char *path);

char * generate_index(const char *path, const char *url, int clientfd);
