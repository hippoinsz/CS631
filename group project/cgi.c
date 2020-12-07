#define _GNU_SOURCE

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cgi.h"

int run_cgi(char *path, char *argv0, int print_newline, int fdin) {
  int pipe[2];
  int status;
  pid_t pid;
  int rpipe;
  int wpipe;

  pipe2(pipe, O_NONBLOCK);
  rpipe = pipe[0];
  wpipe = pipe[1];

  pid = fork();
  if (pid == 0) {
    if (fdin != 0) {
      dup2(fdin, STDIN_FILENO);
    }
    close(rpipe);
    dup2(wpipe, STDOUT_FILENO);
    if (print_newline == 1) {
      write(wpipe, "\r\n", 2);
      execlp(path, path, argv0, NULL);
    } else {
      execl(path, path, argv0, NULL);
    }
  } else {
    waitpid(pid, &status, 0);
    close(wpipe);
  }
  return rpipe;
}
