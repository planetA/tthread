#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "minunit.h"

MU_TEST(test_notify) {
  int fd[2];
  int rc = socketpair(PF_LOCAL, SOCK_SEQPACKET, 0, fd);

  mu_check(rc == 0);
  auto val = std::to_string(fd[1]);
  auto testExecutable = std::string(TEST_PATH) + "/usage-test";

  char temp[256];
  getcwd(temp, 256);
  printf("cwd: %s\n", temp);

  int child = fork();

  switch (child) {
  case -1:
    mu_fail("fail to fork");
    break;

  case 0:
    close(fd[0]);
    mu_check(setenv("TTHREAD_NOTIFY_SOCK", val.c_str(), 1) == 0);
    execl(testExecutable.c_str(), "usage-test", NULL);
    exit(1);
    break;

  default:
    close(fd[1]);
    pid_t pid = 0;
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, '\0', sizeof(buf));
    struct iovec io = { .iov_base = &pid, .iov_len = sizeof(pid), };
    struct msghdr msg = {};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    mu_check(recvmsg(fd[0], &msg, 0) > 0);
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    mu_check(pid == child);

    mu_check(cmsg != NULL);
    mu_check(cmsg->cmsg_type == SCM_RIGHTS);
    int fd = *((int *)CMSG_DATA(cmsg));
    write(fd, "OK", sizeof("OK"));

    int status;
    waitpid(pid, &status, 0);

    break;
  }
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_notify);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
