//
//  util.c
//  sws
//
//  Created by FuBin Mo on 2020/12/6.
//

#include "util.h"

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

int
validate_path(const char *path)
{
    struct stat st;
    if (path == NULL)
        return 0;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int
validate_port(int str)
{
    if (str < 0 || str > 65535)
        return 0;
    return 1;
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

char *replace (const char *url, const char *old_user, const char *new_user) {
    char *result;
    int i, cnt = 0;
    int new_len = strlen(new_user);
    int old_len = strlen(old_user);
    for (i = 0; url[i] != '\0'; i++) {
        if (strstr(&url[i], old_user) == &url[i]) {
            cnt++;
            i += old_len - 1;
        }
    }
    result = (char *)malloc(i + cnt * (new_len - old_len) + 1);
    i = 0;
    while (*url) {
        if (strstr(url, old_user) == url) {
            strcpy(&result[i], new_user);
            i += new_len;
            url += old_len;
        }
        else
            result[i++] = *url++;
    }
    result[i] = '\0';
    return result;
}
