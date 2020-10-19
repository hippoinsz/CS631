//
//  ls.c
//  test
//
//  Created by FuBin Mo on 10/5/20.
//

#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<dirent.h>
#include<fts.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include"print.h"
#include <limits.h>

#ifndef BLOCKSIZE
#define BLOCKSIZE 512
#endif

#define max(a, b) ((a) > (b) ? (a) : (b))

static int Aflag = 0;
static int aflag = 0;
static int cflag = 0;
static int dflag = 0;
static int Fflag = 0;
static int fflag = 0;
static int hflag = 0;
static int iflag = 0;
static int kflag = 0;
static int lflag = 0;
static int nflag = 0;
static int qflag = 0;
static int Rflag = 0;
static int rflag = 0;
static int Sflag = 0;
static int sflag = 0;
static int tflag = 0;
static int uflag = 0;
static int wflag = 0;

static int file_row = 1;

int nlink_maxlen = 0, user_maxlen = 0, group_maxlen = 0, size_maxlen = 0, blksize_maxlen = 0;

//stat str struct
typedef struct stat_str{
    struct stat *filestat;
    char *inode;
    long blocks;
    char *blksize;
    char *fname;
    char *user;
    char *group;
    char size[16];
    char time[13];
    char mode[11];
    char nlink[5];
    struct stat_str *next; //use as linked list
} st_str;

void build_line(const st_str *stat_strbuf, char* fmtstrbuf);

void display_dir (char **files, st_str *str_buf);

void default_display(const st_str *stat_strbuf);

char **get_opt_and_files (int argc, char **argv);

void get_stat_str (struct stat *filestat, char *filename, st_str *str_buf);

void human_readable(off_t bytes, char *szbuf);

void long_display(const st_str *stat_strbuf);

void sort_by_time (st_str *stat_strbuf);

int
main (int argc, char **argv) {
        
    char **files;
    st_str * root_str_buf = (st_str*)(malloc(sizeof(st_str)));
    
    files = get_opt_and_files(argc, argv);
    
    display_dir(files, root_str_buf);
    
    st_str *p = root_str_buf;
    
    if (cflag || tflag || uflag) {
        sort_by_time(p);
    }
    
    while (p->next != NULL) {
        if (iflag) {
            printf("%s ", p->inode);
        }
        if (lflag || nflag){
            long_display(p);
        }else{
            default_display(p);
        }
        
        p = p -> next;
    }
    
    return EXIT_SUCCESS;
}

void build_line(const st_str *stat_strbuf, char* fmtstrbuf){
    char fmt[32];
    if (sflag) {
        snprintf(fmt, 32,"%%%ds %%s %%%ds %%-%ds %%-%ds %%%ds %%s %%s", blksize_maxlen, nlink_maxlen, user_maxlen, group_maxlen, size_maxlen);
        sprintf(fmtstrbuf, fmt, stat_strbuf->blksize, stat_strbuf->mode, stat_strbuf->nlink, stat_strbuf->user,
            stat_strbuf->group, stat_strbuf->size, stat_strbuf->time, stat_strbuf->fname);
    } else {
        snprintf(fmt, 32,"%%s %%%ds %%-%ds %%-%ds %%%ds %%s %%s", nlink_maxlen, user_maxlen, group_maxlen, size_maxlen);
        sprintf(fmtstrbuf, fmt, stat_strbuf->mode, stat_strbuf->nlink, stat_strbuf->user,
            stat_strbuf->group, stat_strbuf->size, stat_strbuf->time, stat_strbuf->fname);
    }
}

void default_display(const st_str *stat_strbuf){
    printf("%s \n", stat_strbuf->fname);
}

void
display_dir (char **files, st_str *str_buf) {
    
    st_str * p = str_buf;
    
    int fts_options = FTS_PHYSICAL;
    
    if (aflag){
        fts_options = FTS_PHYSICAL | FTS_SEEDOT;
    }
    
    FTS* fts_file = fts_open(files, fts_options, NULL);
    
    FTSENT* file_entity = NULL;
    
    while ((file_entity = fts_read(fts_file)) != NULL) {
        
        // Even if the file or directories is not exist, the fts_read can return an entity of it.
        // So we need to judge the fts_info to find out if the file can be open or can be stat.
        if (file_entity->fts_info == FTS_DNR || file_entity->fts_info == FTS_ERR || file_entity->fts_info == FTS_NS) {
            fprintf(stderr, "Read file error is %s: %s \n", file_entity->fts_name,strerror(file_entity->fts_errno));
            exit(EXIT_FAILURE);
        }
        
        // if there is a -d then i just print the level-0 file
        if (dflag) {
            if (file_entity->fts_level != 0) {
                continue;
            }
        }
        else {
            if (Aflag == 0) {
                // if there is no -a or -A, remove the .xxx file
                if((file_entity->fts_name[0]) == '.') {
                    continue;
                }
            }
            
            // if the target file is not a regular file, the fts will return the "." twice when it is the starting point
            // so I will remove the root of the directories.
            if (file_entity->fts_info != FTS_F && file_entity->fts_level == 0) {
                continue;
            }
        }
        
        if (file_entity->fts_info == FTS_DP) {
            continue;
        }
        
        // if there is no -R, we will not need to recursively travel the directories
        if (!Rflag) {
            if (file_entity->fts_level > 1) {
                continue;
            }
        }
        
        
        get_stat_str(file_entity->fts_statp, file_entity->fts_name, p);

        p -> next = (st_str*)(malloc(sizeof(st_str)));
        p = p -> next;
        p -> next = NULL;
    }
}

char **
get_opt_and_files (int argc, char **argv) {
    int opt;
    char *optstring = "AacdFfhiklnqRrSstuw";
    
    char **files;
    int index = 1;
    
    int wIndex = 0;
    int qIndex = 0;
    int lIndex = 0;
    int nIndex = 0;
    int cIndex = 0;
    int uIndex = 0;
    int hIndex = 0;
    int kIndex = 0;
    
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'A':
                Aflag = 1;
                break;
            case 'a':
                aflag = 1;
                Aflag = 1;
                break;
            case 'c':
                cflag = 1;
                cIndex = optind;
                break;
            case 'd':
                dflag = 1;
                break;
            case 'F':
                Fflag = 1;
                break;
            case 'f':
                fflag = 1;
                break;
            case 'h':
                hflag = 1;
                hIndex = optind;
                break;
            case 'i':
                iflag = 1;
                break;
            case 'k':
                kflag = 1;
                kIndex = optind;
                break;
            case 'l':
                lflag = 1;
                lIndex = optind;
                break;
            case 'n':
                nflag = 1;
                nIndex = optind;
                break;
            case 'q':
                qflag = 1;
                qIndex = optind;
                break;
            case 'R':
                Rflag = 1;
                break;
            case 'r':
                rflag = 1;
                break;
            case 'S':
                Sflag = 1;
                break;
            case 's':
                sflag = 1;
                break;
            case 't':
                tflag = 1;
                break;
            case 'u':
                uflag = 1;
                uIndex = optind;
                break;
            case 'w':
                wflag = 1;
                wIndex = optind;
                break;
                
            case '?':
                fprintf(stderr, "Wrong opt is optopt %d: %s \n", optopt, strerror(errno));
                exit(EXIT_FAILURE);
                
            default:
                break;
        }
        
        // get the last opt index that the rest of argv is the directories
        index = optind;
    }
    
    // Override the conflict flags
    if (hIndex > kIndex) {
        hflag = 1;
        kflag = 0;
    } else if (hIndex < kIndex) {
        hflag = 0;
        kflag = 1;
    }
    
    if (wIndex > qIndex) {
        wflag = 1;
        qflag = 0;
    } else if (wIndex < qIndex) {
        wflag = 0;
        qflag = 1;
    }
    
    if (lIndex > nIndex) {
        lflag = 1;
        nflag = 0;
    } else if (lIndex < nIndex) {
        lflag = 0;
        nflag = 1;
    }
    
    if (cIndex > uIndex) {
        cflag = 1;
        uflag = 0;
    } else if (cIndex < uIndex) {
        cflag = 0;
        uflag = 1;
    }
    
    // The user may pass multiple directories, the rest of argv is the directories
    if (index < argc) {
        file_row = argc - index;
        files = argv + index;
    }
    // if the user do not pass directories, default list the current folder
    else {
        files = (char **) malloc(sizeof(char *) * file_row);
        if (files == NULL) {
            fprintf(stderr, "Uable to malloc to files space: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        files[0] = ".";
    }
    
    return files;
}

void
get_stat_str (struct stat *filestat, char *filename, st_str *str_buf) {
    
    //char mode[11];
    // set stat
    str_buf->filestat = filestat;
    
    // set inode
    str_buf->inode = malloc(sizeof(filestat->st_ino));
    snprintf(str_buf->inode, sizeof(filestat->st_ino), "%ld", filestat->st_ino);
    
    // set mode
    snprintf(str_buf->mode, 11, "%c%c%c%c%c%c%c%c%c%c",
        S_ISREG(filestat->st_mode) ? '-' : (
            S_ISDIR(filestat->st_mode) ? 'd' : (
                S_ISBLK(filestat->st_mode) ? 'b' : (
                    S_ISCHR(filestat->st_mode) ? 'c' : (
                        S_ISLNK(filestat->st_mode) ? 'l' : (
                            S_ISSOCK(filestat->st_mode) ? 's': (
                                S_ISFIFO(filestat->st_mode) ? 'p' : 'w'
                                                                )
                                                            )
                                                        )
                                                    )
                                                )
        ),
        (S_IRUSR & filestat->st_mode ) ? 'r' : '-',
        (S_IWUSR & filestat->st_mode ) ? 'w' : '-',
        (S_IXUSR & filestat->st_mode ) ? 'x' : '-',
        (S_IRGRP & filestat->st_mode ) ? 'r' : '-',
        (S_IWGRP & filestat->st_mode ) ? 'w' : '-',
        (S_IXGRP & filestat->st_mode ) ? 'x' : '-',
        (S_IROTH & filestat->st_mode ) ? 'r' : '-',
        (S_IWOTH & filestat->st_mode ) ? 'w' : '-',
        (S_IXOTH & filestat->st_mode ) ? 'x' : '-'
    );
    
    //set nlink
    snprintf(str_buf->nlink, 5, "%hu", filestat->st_nlink);
    nlink_maxlen = max(nlink_maxlen, (int)strlen(str_buf->nlink));
    
    //user, group
    if (nflag == 1) {
        
        str_buf->user = malloc(sizeof(filestat->st_uid));
        if (str_buf->user == NULL) {
            fprintf(stderr, "Uable to malloc to str_buf->st_uid space: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        str_buf->group = malloc(sizeof(filestat->st_gid));
        if (str_buf->group == NULL) {
            fprintf(stderr, "Uable to malloc to str_buf->st_gid space: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        snprintf(str_buf->user, sizeof(filestat->st_uid), "%u", filestat->st_uid);
        snprintf(str_buf->group, sizeof(filestat->st_gid), "%u", filestat->st_gid);
    } else {
        
        char * uname = getpwuid(filestat->st_uid)->pw_name;
        char * gname = getgrgid(filestat->st_gid)->gr_name;
        str_buf->user = malloc(strlen(uname));
        if (str_buf->user == NULL) {
            fprintf(stderr, "Uable to malloc to str_buf->st_uid space: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        str_buf->group = malloc(strlen(gname));
        if (str_buf->group == NULL) {
            fprintf(stderr, "Uable to malloc to str_buf->st_gid space: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        snprintf(str_buf->user, 20, "%s", uname);
        snprintf(str_buf->group, 20, "%s", gname);
    }
    
    user_maxlen = max(user_maxlen, (int)strlen(str_buf->user));
    group_maxlen = max(group_maxlen, (int)strlen(str_buf->group));
    
    //size
    long size = filestat->st_size;
    if(hflag || kflag){
        char szbuf[16];
        human_readable(size, szbuf);
        snprintf(str_buf->size, 16,"%s", szbuf);
    } else {
        snprintf(str_buf->size, 16, "%ld", size);
    }
    size_maxlen = max(size_maxlen, (int)strlen(str_buf->size));
    
    //set and blksize
    str_buf->blksize = malloc(sizeof(size / BLOCKSIZE));
    snprintf(str_buf->blksize, sizeof(size / BLOCKSIZE), "%ld", (size/BLOCKSIZE));
    blksize_maxlen = max(blksize_maxlen, (int)strlen(str_buf->blksize));
    
    //time
    strftime(str_buf->time, 13, "%b %d %H:%M", localtime(&(filestat->st_mtime)));
    
    // executable flag
    int pwd = getuid();
    int gid = getgid();
    int eflag = 0;
    
    if (pwd == (int)filestat->st_uid && S_IXUSR & filestat->st_mode) {
        eflag = 1;
    } else if (gid == (int)filestat->st_gid && S_IXGRP & filestat->st_mode) {
        eflag = 1;
    } else if (S_IXOTH & filestat->st_mode){
        eflag = 1;
    }
    
    if (Fflag) {
        strncat(filename, (str_buf->mode)[0] == 'd' ? "/" : (
                               (str_buf->mode)[0] == 'l' ? "@" : (
                                    (str_buf->mode)[0] == 's' ? "=" : (
                                         str_buf->mode)[0] == 's' ? "%" : (
                                             (str_buf->mode)[0] == 'p' ? "|" : (
                                                    eflag ? "*" : " ")
                                                                )
                                                            )
                                                        ), 1);
    }
    
    str_buf->fname = malloc(strlen(filename)+1);
    if (str_buf->fname == NULL) {
        fprintf(stderr, "Uable to malloc to str_buf->fname space: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    snprintf(str_buf->fname, strlen(filename)+1, "%s", filename);

}

void
human_readable(off_t nbytes, char *szbuf){
    if (kflag){
        sprintf(szbuf, "%.1lfK", (double)nbytes / 1024);
    } else {
        if(nbytes < 1024)
            sprintf(szbuf, "%ldB", nbytes);
        else if(nbytes < 1024 * 1024)
            sprintf(szbuf, "%.1lfK", (double)nbytes / 1024);
        else if(nbytes < 1024 * 1024 * 1024)
            sprintf(szbuf, "%.1lfM", (double)nbytes / 1024 / 1024);
        else
            sprintf(szbuf, "%.1lfG", (double)nbytes / 1024 / 1024 / 1024);
    }
}

void
long_display (const st_str *stat_strbuf) {
    char fmtstrbuf[256];
    build_line(stat_strbuf, fmtstrbuf);
    puts(fmtstrbuf);
}

// Insertsort
void
sort_by_time (st_str *stat_strbuf) {

    st_str * p1 = stat_strbuf;
    int index = 0;
    while(p1->next != NULL){

        st_str *min_buf = p1;
        printf("the file1 is %s. \n", p1->fname);
        printf("his next file is %s. \n", p1->next->fname);
        long mintime;
        if (uflag) {
            mintime = p1->filestat->st_atime;
        } else if (cflag) {
            mintime = p1->filestat->st_ctime;
        } else {
            mintime = p1->filestat->st_mtime;
        }

        st_str * p2 = p1 -> next;
        while (p2->next != NULL) {
            printf("the file2 is %s. \n", p2->fname);
            long time2;
            if (uflag) {
                time2 = p2->filestat->st_atime;
            } else if (cflag) {
                time2 = p2->filestat->st_ctime;
            } else {
                time2 = p2->filestat->st_mtime;
            }
            if (time2 < mintime) {
                mintime = time2;
                min_buf = p2;
            }
            p2 = p2->next;
            printf("the min file is %s. \n", min_buf->fname);
        }

        //swap the min stat;
        st_str * temp = p1;
        st_str * mintemp = min_buf;
        
        printf("the %d min time file is %s.", index, min_buf->fname);
        p1 = min_buf;
        p1->next = temp->next;
        
        min_buf = temp;
        min_buf->next = mintemp->next;

        p1 = p1->next;
        index++;
    }
    printf("total has %d loops.", index);
}
