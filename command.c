//
//  command.c
//  
//
//  Created by FuBin Mo on 11/2/20.
//

#include <sys/wait.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
command (char *string, char *outbuf, int outlen, char *errbuf, int errlen) {
    int fdout[2], fderr[2];
    char *execve_str[] = {"sh", "-c", NULL, NULL};
    char *env[] = {"PATH=/bin", 0};
    sigset_t set;
    pid_t pid;
    
    execve_str[2] = string;
    
    if (pipe(fdout) < 0) {
        fprintf(stderr, "Unable to create stdout pipe: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    
    if (pipe(fderr) < 0) {
        fprintf(stderr, "Unable to create stderr pipe: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    
    if(signal(SIGINT,SIG_IGN) == SIG_ERR || signal(SIGQUIT,SIG_IGN) == SIG_ERR){
        fprintf(stderr,"SIGINT or SIGQUIT ignore error: %s\n",strerror(errno));
        return EXIT_FAILURE;
    }
    
    if (sigemptyset(&set) < 0) {
        fprintf(stderr,"sigemptyset error: %s\n",strerror(errno));
        return EXIT_FAILURE;
    }
    
    if (sigaddset(&set, SIGCHLD) < 0 ) {
        fprintf(stderr,"sigaddset SIGCHLD to set error: %s\n",strerror(errno));
        return EXIT_FAILURE;
    }
    
    if(sigprocmask(SIG_BLOCK, &set, NULL) < 0){
        fprintf(stderr,"SIG_BLOCK SIGCHLD error:%s\n",strerror(errno));
        return EXIT_FAILURE;
    }
    
    if((pid = fork()) < 0) {
        fprintf(stderr, "Unable to fork a child process: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    else if (pid == 0) {
        // in the child process
        (void)close(fdout[0]);
        (void)close(fderr[0]);
        
        if(dup2(fdout[1], STDOUT_FILENO) < 0) {
            fprintf(stderr, "dup2 outfd failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        if(dup2(fderr[1], STDERR_FILENO) < 0) {
            fprintf(stderr, "dup2 errfd failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        execve(_PATH_BSHELL, execve_str, env);
        (void)close(fdout[1]);
        (void)close(fderr[1]);
    } else {
        // in the parent process
        (void)close(fdout[1]);
        (void)close(fderr[1]);
        
        if(read(fdout[0],outbuf,outlen) == -1){
            fprintf(stderr,"error while reading stdout:%s\n",strerror(errno));
            return EXIT_FAILURE;
        }

        if(read(fderr[0],errbuf,errlen) == -1){
            fprintf(stderr,"error while reading stderr:%s\n",strerror(errno));
            return EXIT_FAILURE;
        }
        
        wait(NULL);
    }
    
    return EXIT_SUCCESS;
}

int
main() {
    char out[BUFSIZ], err[BUFSIZ];
    if (command("ls -l", out, BUFSIZ, err, BUFSIZ) == -1) {
        perror("command");
        exit(EXIT_FAILURE);
    }
    printf("stdout:\n%s\n", out);
    printf("stderr:\n%s\n", err);
    return EXIT_SUCCESS;
}
