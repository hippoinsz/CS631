//
//  server.c
//  sws
//
//  Created by FuBin Mo on 2020/11/19.
//

#include "server.h"

#define LISTENSIZE 5
#define CONTENTBUF 20000

#define BAD_REQUEST "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>Your browser sent a request that this server could not understand.<br />\n</p>\n</body></html>\n"
#define FORBIDDEN "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>403 Forbidden</h1>\n<p>Your browser sent a request that this server forbid.<br />\n</p>\n</body></html>\n"
#define NOT_FOUND "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>404 Not Found</h1>\n<p>Your browser sent a request that this server could not find.<br />\n</p>\n</body></html>\n"
#define SERVER_ERROR "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>500 Internal Server Error</title>\n</head><body>\n<h1>500 Internal Server Error</h1>\n<p>Your browser sent a request that this server get error.<br />\n</p>\n</body></html>\n"
#define NOT_MODIFIED "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>304 Not Modified</title>\n</head><body>\n<h1>304 Not Modified</h1>\n<p>Your browser sent a request that the file is not modified.<br />\n</p>\n</body></html>\n"
#define STR_HTML_DOCTYPE "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>"

#define CONTENT "Content-Type: text/html\r\n"
#define CONNECT_SUCCESS "HTTP/1.0 200 OK\r\n"
#define SERVER_STRING "Server: sws 1.0\r\n"

#define TIMESIZ BUFSIZ

char * sws;
int log_fd;
const char *cgi_path = NULL;

void deal_network(u_short *port, const char *ip, char *sws_dir, int fd,
                  const char *cgi_dir) {
    int listenedfd, listenedfd_ipv6;
    int clientfd;
    struct sockaddr_in client;
    
    sws = sws_dir;
    log_fd = fd;
    
    if (cgi_dir != NULL) {
        cgi_path = cgi_dir;
    }
    
    if (i_flag == 1) {
        if (is_valid_ipv4(ip))
            listenedfd = createIpv4Socket(port, ip);
        else if (is_valid_ipv6(ip))
            listenedfd = createIpv6Socket(port, ip);
        else{
            perror("no vaild ip to create server");
            exit(EXIT_FAILURE);
        }
    }else {
        listenedfd = createIpv4Socket(port, ip);
        listenedfd_ipv6 = createIpv6Socket(port, ip);
    }
    
    int client_len = sizeof(client);
    /* use a new thread to listen ipv6 */
    pthread_t ipv6_thread, client_thread;
    if (pthread_create(&ipv6_thread, NULL, accept_ipv6, (void*)&listenedfd_ipv6) != 0)
        perror("create ipv6 error");
    while (1) {
        if((clientfd = accept(listenedfd, (struct sockaddr *)&client, (socklen_t *)&client_len)) == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        if (log_fd > 0 && l_flag == 1)
            write_log(clientfd, log_fd);
        if(d_flag)
            handle_request(&clientfd);
        else { /* create a thread to handle 2 request */
            if(pthread_create(&client_thread, NULL, handle_request, &clientfd) != 0)
                perror("create thread error");
        }
    }
    
    close(listenedfd);
}

void
*accept_ipv6(void* listened){
    int listenedfd = *(int *)listened;
    struct sockaddr_in6 client;
    int client_len = sizeof(client);
    int clientfd;
    pthread_t client_thread;
    while (1) {
        if((clientfd = accept(listenedfd, (struct sockaddr *)&client, (socklen_t *)&client_len)) == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        if(d_flag)
            handle_request(&clientfd);
        else {
            if(pthread_create(&client_thread, NULL, handle_request, &clientfd) != 0)
                perror("create thread error");
        }
    }
    close(listenedfd);
}

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
get_request_info(int socket, char *buf, int size){
    int i = 0;
    int len = 0;
    char c = '\0';
    while (i < size - 1 && (c !='\n')) {
        len = recv(socket, &c, 1, 0);
        if (len > 0) {
            if(c == '\r'){
                len = recv(socket, &c, 1, MSG_PEEK); 
                if((len > 0) && (c == '\n'))
                    recv(socket, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            ++i;
        }else
            c = '\n';
    }
    buf[i] = '\0';
    return len;
}

void *
handle_request(void* client) {
    int clientfd = *(int *)client;
    char buf[BUFSIZ];
    char method[BUFSIZ];
    char url[BUFSIZ];
    char *path = NULL;
    char temp[BUFSIZ];
    char modify[BUFSIZ];
    char http_version[BUFSIZ];
    char *query_string = NULL;
    int i = 0, j = 0;
    int n = get_request_info(clientfd, buf, BUFSIZ);
    if (log_fd > 0 && l_flag == 1){
        write(log_fd, buf, strlen(buf) - 1);
    }
    struct stat st;
    while(!isspace(buf[i]) && i < BUFSIZ - 1){
        method[i] = buf[i];
        i++;
    }
    method[i] = '\0';
    printf("method: %s\n",method);
    while (isspace(buf[i])) {
        i++;
    }
    while (!isspace(buf[i]) && j < BUFSIZ - 1) {
        url[j] = buf[i];
        j++;
        i++;
    }
    url[j] = '\0';
    printf("url :%s\n",url);
    query_string = url;
    // handle_dir
    if ((stat(sws, &st)) == -1) {
        send_error(clientfd, NOT_FOUND, sws);
        return NULL;
    }
    
    path = (char *)malloc((size_t)BUFSIZ);
    if(getcwd(path, BUFSIZ + 1) == NULL){
        send_error(clientfd, NOT_FOUND, path);
        return NULL;
    }
    
    // url begin with /~ use replace to get new user
    if (query_string[1] == '~') {
        char path_buf[BUFSIZ];
        char new_user[BUFSIZ];
        char *p = NULL;
        char *q = NULL;

        query_string+=2;
        
        memset(path_buf,0,sizeof(path_buf));
        strcpy(new_user,query_string);
        q = strstr(query_string, "/");
        p = strtok(new_user, "/");
        p = getenv("USER");
        if (p)
            path = replace(path, p, new_user);
        strcat(path,q);
    }else
        strcat(path,url);
    printf("path: %s\n",path);

    while (isspace(buf[i]))
        i++;
    j = 0;
    while (!isspace(buf[i]) && j < BUFSIZ - 1) {
        http_version[j] = buf[i];
        j++;
        i++;
    }
    http_version[j] = '\0';
    printf("version:%s\n",http_version);
    // not GET and HEAD method
    if (strcmp(method, "GET") && strcmp(method, "HEAD")) {
        send_error(clientfd, BAD_REQUEST, path);
        return NULL;
    }

    if ((n = strcmp(http_version, "HTTP/1.0")) !=0) {
        send_error(clientfd, BAD_REQUEST, path);
        return NULL;
    }
    // get if-modify-scince timestamp
    while ((n = strcmp(buf, "\n")) != 0 ) {
        i = 0;
        j = 0;
        get_request_info(clientfd, buf, sizeof(buf));
        while (isspace(buf[i]))
            i++;
        while (!isspace(buf[i]) && j < BUFSIZ - 1) {
            temp[j] = buf[i];
            i++;
            j++;
        }
        temp[j] = '\0';
        if ((n = strcmp(temp, "If-Modified-Since:")) == 0){
            strcpy(modify,buf);
            printf("%s\n",modify);
        }
    }

    if ((n = strcmp(method, "HEAD")) == 0)
        handle_head(clientfd, path);

    if((n = strcmp(method, "GET")) == 0){
        handle_get(clientfd, path, modify);
        bzero(modify, sizeof(modify));
    }

    close(clientfd);
    return NULL;
}

void handle_head(int clientfd, const char *path){
    int fd;
    
    if ((fd = open(path, O_RDONLY)) < 0){
        send_error(clientfd, FORBIDDEN, path);
    } else {
        send_success(clientfd, path);
    }
}

void handle_get(int clientfd, const char *path, const char *modify){
    int dp, n, c;
    char buf[TIMESIZ];
    char *content_buf;
    char *home = NULL;
    char abs_path[BUFSIZ];
    struct stat st;
    struct stat sp;
    struct tm ts;
    // abs_path should not out of with home/
    home = (char *)malloc((size_t)BUFSIZ);
    realpath(path, abs_path);
    home = "/home";
    
    char *npath = (char *)malloc(strlen(path)+1);
    strcpy(npath, path);
    
    if (c_flag && strcmp(basename(npath), cgi_path) == 0) {
        /* Test Existence */
        if (access(path, R_OK) != 0) {
            /* Not Exists*/
            /* Or permission denied*/
            send_error(clientfd, FORBIDDEN, path);
            return;
        } else if (access(path, X_OK) != 0) {
            /* Not Executable*/
            send_error(clientfd, FORBIDDEN, path);
            return;
        } else {
            send_success(clientfd, path);
            run_cgi(path, NULL, 0, clientfd);
        }
    }
    
    // the path should not get out of /home or /Users
    if ((n = strncmp(abs_path, home, strlen(home))) != 0 ) {
        send_error(clientfd, FORBIDDEN, path);
        return;
    }
    if ((dp = open(path, O_RDONLY)) < 0){
        send_error(clientfd, FORBIDDEN, path);
        close(dp);
        return;
    }
    if ((stat(path, &st)) == -1) {
        send_error(clientfd, NOT_FOUND, path);
        return;
    }
    ts = *gmtime((time_t*)&st.st_mtimespec);
    strftime(buf, sizeof(buf), "If-Modified-Since: %a, %d %b %Y %H:%M:%S GMT\n", &ts);
    if ((n = strcmp(buf, modify)) == 0) {
        send_error(clientfd, NOT_MODIFIED, path);
        return;
    }else {
        send_success(clientfd, path);
    }
    // For request a directory
    if (S_ISDIR(st.st_mode)) {
        char temp[BUFSIZ];
        strcpy(temp,path);
        c = (int)strlen(temp) - 1;
        temp[c] == '/' ? strcat(temp, "index.html") : strcat(temp, "/index.html");
        
        // if there has an index.html
        if (stat(temp, &sp) == 0) {
            send(clientfd, buf, strlen(buf), 0);
            if (log_fd > 0 && l_flag == 1)
                write(log_fd,buf, strlen(buf));
            send(clientfd,"\r\n",strlen("\r\n"),0);
            close(dp);
            dp = open(temp, O_RDONLY);
            memset(buf,0,sizeof(buf));
            while ((n = read(dp, buf, BUFSIZ)) > 0)
                send(clientfd, buf,strlen(buf),0);
            send(clientfd,"\r\n",strlen("\r\n"),0);
            close(dp);
            return;
        }
        
        // if not have index
        content_buf = generate_index(path, abs_path, clientfd);
        
        sprintf(buf,"Content-Length: %d\r\n",(int)strlen(content_buf));
        send(clientfd, buf, strlen(buf), 0);
        if (log_fd > 0 && l_flag == 1)
            write(log_fd, buf, strlen(buf));
        send(clientfd,"\r\n",strlen("\r\n"),0);
        send(clientfd, content_buf, strlen(content_buf), 0);
    }
    // For request a file
    else {
        sprintf(buf,"Content-Length: %d\r\n",(int)st.st_size);
        send(clientfd, buf, strlen(buf), 0);
        if (log_fd > 0 && l_flag == 1)
            write(log_fd, buf, strlen(buf));
        send(clientfd,"\r\n",strlen("\r\n"),0);
        memset(buf,0,sizeof(buf));
        while ((n = read(dp, buf, BUFSIZ)) > 0)
            send(clientfd, buf,strlen(buf),0);
        send(clientfd,"\r\n",strlen("\r\n"),0);
        close(dp);
    }
}

int
formate_date(int clientfd){
    time_t     now;
    struct tm  ts;
    char       buf[TIMESIZ];
    time(&now);
    ts = *gmtime(&now);
    strftime(buf, sizeof(buf), "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", &ts);
    send(clientfd, buf, strlen(buf), 0);
    return 0;
}

int
send_timestamp(int clientfd, const char *path){
    char buf[TIMESIZ];
    struct stat st;
    struct tm ts;
    if ((stat(path, &st)) == -1) {
        perror("stat error");
        close(clientfd);
        return EXIT_FAILURE;
    }
    ts = *gmtime((time_t*)&st.st_mtimespec);
    strftime(buf, sizeof(buf), "Last-Modified: %a, %d %b %Y %H:%M:%S GMT\r\n", &ts);
    send(clientfd, buf, strlen(buf), 0);
    return EXIT_SUCCESS;
}

void
send_error(int clientfd, char *str, const char *path) {
    send(clientfd, str, strlen(str), 0);
    if (log_fd > 0 && l_flag == 1)
        write(log_fd, str, strlen(str) - 2);
    formate_date(clientfd);
    send(clientfd, SERVER_STRING, strlen(SERVER_STRING), 0);
    send_timestamp(clientfd, path);
    send(clientfd, "Content-Length: 0\r\n", strlen(str) + strlen(SERVER_STRING) + strlen("Content-Length: 0\r\n"), 0);
    if (log_fd > 0 && l_flag == 1)
        write(log_fd, " Content-Length: 0\n", strlen(str) + strlen(SERVER_STRING) + strlen("Content-Length: 0\r\n"));
    close(clientfd);
}

void
send_success(int clientfd, const char *path) {
    send(clientfd, CONNECT_SUCCESS, strlen(CONNECT_SUCCESS), 0);
    formate_date(clientfd);
    send(clientfd, SERVER_STRING, strlen(SERVER_STRING), 0);
    send_timestamp(clientfd, path);
}

static const char *
get_time_str(time_t ftime)
{
        static char buffer[32];
        strftime(buffer, 32, "%F", localtime(&ftime));
        return buffer;
}

char *
generate_index(const char *path, const char *url, int clientfd)
{
    char *index_html, *pos;
    int left, count;
    struct dirent **dirlist;
    struct dirent *dp;
    int dircount, dirindex;
    struct stat sb;

    if ((index_html = (char *)malloc(CONTENTBUF)) != NULL) {
        pos = index_html;
        left = CONTENTBUF;

        if ((count = snprintf(pos, left, STR_HTML_DOCTYPE)) < 0) {
            send_error(clientfd, SERVER_ERROR, path);
            return NULL;
        } else {
            pos += count;
            left -= count;
        }

        if ((count = snprintf(pos, left, "<head><title>Path %s</title></head><body>Path %s<div><table class='list'><tr class='nohover'><th class='left'>Name</th><th class='right'>Size</th><th class='left'>Last Modified</th></tr>",
                        url, url)) < 0) {
            send_error(clientfd, SERVER_ERROR, path);
            free(index_html);
            return NULL;
        } else {
            pos += count;
            left -= count;
        }

        dircount = scandir(path, &dirlist, 0, alphasort);
        if (dircount < 0) {
            send_error(clientfd, SERVER_ERROR, path);
            free(index_html);
            return NULL;
        } else {
            for (dirindex = 0; dirindex < dircount; ++dirindex) {
                dp = dirlist[dirindex];
                lstat(dp->d_name, &sb);
                if (dp->d_name[0] != '.') {
                    if (url[strlen(url) - 1] == '/')
                        count = snprintf(pos, left, "<tr><td><a href='%s%s'>%s</a></td><td>%lu</td><td>%s</td></tr>",
                                url, dp->d_name, dp->d_name, (long unsigned int)sb.st_size, get_time_str(sb.st_mtime));
                    else
                        count = snprintf(pos, left, "<tr><td><a href='%s/%s'>%s</a></td><td>%lu</td><td>%s</td></tr>",
                                url, dp->d_name, dp->d_name, (long unsigned int)sb.st_size, get_time_str(sb.st_mtime));
                    pos += count;
                    left -= count;
                }
                free(dirlist[dirindex]);
            }
            free(dirlist);
        }

        if ((count = snprintf(pos, left, "</table></div></body></html>\n")) < 0) {
            send_error(clientfd, SERVER_ERROR, path);
            free(index_html);
            return NULL;
        }
        index_html[CONTENTBUF - 1] = '\0';
    }
    return index_html;
}


void
send_content(int clientfd, const char *path){
    char buf[BUFSIZ];
    const char *mime;
    magic_t magic;
    magic = magic_open(MAGIC_MIME_TYPE);
    magic_load(magic, NULL);
    mime = magic_file(magic,path);
    sprintf(buf, "Content-Type: %s\r\n",mime);
    send(clientfd, buf, strlen(buf) , 0);
    magic_close(magic);
}
