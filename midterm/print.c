//
//  print.c
//  test
//
//  Created by FuBin Mo on 10/18/20.
//

#include "print.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

//int nlink_maxlen = 0, user_maxlen = 0, group_maxlen = 0, size_maxlen = 0;

//void human_readable(off_t nbytes, char *szbuf);
//
//void build_line(const st_str *stat_strbuf, char* fmtstrbuf){
//    printf("the length is %d", nlink_maxlen+user_maxlen+group_maxlen+size_maxlen);
//    char fmt[100];
//    snprintf(fmt, 100,"%%s %%%ds %%-%ds %%-%ds %%%ds %%s %%s", nlink_maxlen, user_maxlen, group_maxlen, size_maxlen);
//
//    sprintf(fmtstrbuf, fmt, stat_strbuf->mode, stat_strbuf->nlink, stat_strbuf->user,
//        stat_strbuf->group, stat_strbuf->size, stat_strbuf->time, stat_strbuf->fname);
//
//}
//
//void
//default_display (const st_str *stat_strbuf) {
//    char fmtstrbuf[256];
//    build_line(stat_strbuf, fmtstrbuf);
//    //puts(fmtstrbuf);
//}
//
//void
//get_stat_str (st_str *str_buf, int nflag, int hflag) {
//
//    //char mode[11];
//    // set stat
//    //str_buf->filestat = stat_buf;ws
//    // set mode
//    snprintf(str_buf->mode, 11, "%c%c%c%c%c%c%c%c%c%c",
//        S_ISREG(str_buf->filestat->st_mode) ? '-' : (
//            S_ISDIR(str_buf->filestat->st_mode) ? 'd' : (
//                S_ISBLK(str_buf->filestat->st_mode) ? 'b' : (
//                    S_ISCHR(str_buf->filestat->st_mode) ? 'c' : (
//                        S_ISLNK(str_buf->filestat->st_mode) ? 'l' : (
//                            S_ISSOCK(str_buf->filestat->st_mode) ? 's': (
//                                S_ISFIFO(str_buf->filestat->st_mode) ? 'p' : 'w'
//                                                                )
//                                                            )
//                                                        )
//                                                    )
//                                                )
//        ),
//        (S_IRUSR & str_buf->filestat->st_mode ) ? 'r' : '-',
//        (S_IWUSR & str_buf->filestat->st_mode ) ? 'w' : '-',
//        (S_IXUSR & str_buf->filestat->st_mode ) ? 'x' : '-',
//        (S_IRGRP & str_buf->filestat->st_mode ) ? 'r' : '-',
//        (S_IWGRP & str_buf->filestat->st_mode ) ? 'w' : '-',
//        (S_IXGRP & str_buf->filestat->st_mode ) ? 'x' : '-',
//        (S_IROTH & str_buf->filestat->st_mode ) ? 'r' : '-',
//        (S_IWOTH & str_buf->filestat->st_mode ) ? 'w' : '-',
//        (S_IXOTH & str_buf->filestat->st_mode ) ? 'x' : '-'
//    );
//
//    //set nlink
//    snprintf(str_buf->nlink, 5, "%hu", str_buf->filestat->st_nlink);
//    nlink_maxlen = max(nlink_maxlen, (int)strlen(str_buf->nlink));
//
//    //user, group
//    if (nflag == 1) {
//        snprintf(str_buf->user, 20, "%u", str_buf->filestat->st_uid);
//        snprintf(str_buf->group, 20, "%u", str_buf->filestat->st_gid);
//    } else {
//        snprintf(str_buf->user, 20, "%s", getpwuid(str_buf->filestat->st_uid)->pw_name);
//        snprintf(str_buf->group, 20, "%s", getgrgid(str_buf->filestat->st_gid)->gr_name);
//    }
//
//    user_maxlen = max(user_maxlen, (int)strlen(str_buf->user));
//    group_maxlen = max(group_maxlen, (int)strlen(str_buf->group));
//
//    //size
//    if(hflag == 1){
//        char szbuf[16];
//        human_readable(str_buf->filestat->st_size, szbuf);
//        snprintf(str_buf->size, 16,"%s", szbuf);
//    } else {
//        snprintf(str_buf->size, 16, "%lld", str_buf->filestat->st_size);
//    }
//    size_maxlen = max(size_maxlen, (int)strlen(str_buf->size));
//
//    //time
//    strftime(str_buf->time, 13, "%b %d %H:%M", localtime(&(str_buf->filestat->st_mtime)));
//
//    //default_display(str_buf);
//
//}
//
//void
//human_readable(off_t nbytes, char *szbuf){
//    if(nbytes < 1024)
//        sprintf(szbuf, "%lldB", nbytes);
//    else if(nbytes < 1024 * 1024)
//        sprintf(szbuf, "%.1lfK", (double)nbytes / 1024);
//    else if(nbytes < 1024 * 1024 * 1024)
//        sprintf(szbuf, "%.1lfM", (double)nbytes / 1024 / 1024);
//    else
//        sprintf(szbuf, "%.1lfG", (double)nbytes / 1024 / 1024 / 1024);
//}
