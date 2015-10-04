#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syscall.h>
#include <unistd.h>

#include "notify.h"

int getNotifySock() {
  char *var = getenv("TTHREAD_NOTIFY_SOCK");

  if (var == NULL) {
    return 0;
  }
  int fd = atoi(var);

  if (fd <= 0) {
    fprintf(stderr, "TTHREAD_NOTIFY_SOCK is not a valid number: %s\n", var);
    return -1;
  }

  struct stat statbuf;
  fstat(fd, &statbuf);

  if (!S_ISSOCK(statbuf.st_mode)) {
    fprintf(stderr, "TTHREAD_NOTIFY_SOCK is not a socket\n");
    return -1;
  }

  return fd;
}

int sendFd(int socket, pid_t pid, int fd) {
  struct iovec io;

  io.iov_base = &pid;
  io.iov_len = sizeof(pid_t);

  struct msghdr msg = {};
  msg.msg_iov = &io;
  msg.msg_iovlen = 1;
  char buf[CMSG_SPACE(sizeof(fd))];

  memset(buf, '\0', sizeof(buf));
  msg.msg_control = buf;
  msg.msg_controllen = sizeof(buf);

  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

  *((int *)CMSG_DATA(cmsg)) = fd;

  msg.msg_controllen = cmsg->cmsg_len;

  return sendmsg(socket, &msg, 0);
}

bool notifyParent() {
  int fd = getNotifySock();

  if (fd <= 0) {
    return false;
  }

  pid_t pid = syscall(SYS_getpid);
  int control_fds[2];

  if (pipe(control_fds) != 0) {
    perror("cannot open pipe for TTHREAD_NOTIFY_SOCK");
  }

  if (sendFd(fd, pid, control_fds[1]) <= 0) {
    perror("failed to send file descriptor to TTHREAD_NOTIFY_SOCK");
    close(control_fds[0]);
    close(control_fds[1]);
    return false;
  }
  close(control_fds[1]);

  if (fcntl(control_fds[0], F_SETFD, FD_CLOEXEC) == -1) {
    perror("failed to pipe to CLOEXEC");
  }

  char buf[3];
  int rc = read(control_fds[0], &buf, sizeof(buf));

  if (rc != sizeof(buf)) {
    perror("failed to sync up with pipe from TTHREAD_NOTIFY_SOCK");
    return false;
  }
  return true;
}
