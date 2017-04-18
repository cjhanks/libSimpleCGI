#include "FcgiPreFork.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <cassert>
#include "SimpleCGI/fcgi/FcgiServer.hpp"
#include "SimpleCGI/common/Logging.hpp"


namespace fcgi {
namespace prefork {
namespace {
void
childHangupSignal(int);

void
parentHangupSignal(int);

void
spawnChildEventLoop();

enum Pipe {
  READ  = 0,
  WRITE   = 1
};

static struct {
  ServerConfig config;
  MasterServer* master;
  int dgram_pipe[2];
} GlobalContext;

} // ns

void
eventLoop(MasterServer* master, ServerConfig config, int acceptSock)
{
  ::signal(SIGPIPE, SIG_IGN);

  assert(master != nullptr);
  assert(config.concurrencyModel ==
       ServerConfig::ConcurrencyModel::PREFORKED);
  assert(config.childCount > 0);
  assert(acceptSock >= 0);

  GlobalContext.master = master;
  GlobalContext.config = config;
  if (socketpair(AF_LOCAL, SOCK_STREAM, 0, GlobalContext.dgram_pipe) < 0) {
    assert(1 == 0);
  }

  for (size_t i = 0; i < config.childCount; ++i) {
    spawnChildEventLoop();
  }

  // -
  signal(SIGCHLD, childHangupSignal);

  struct sockaddr_in  addr;
  struct cmsghdr*   cmsg;
  struct msghdr     msg;
  struct iovec    iov;
  union {
    struct cmsghdr cmsghdr;
    char       control[CMSG_SPACE(sizeof(int))];
  } cmsg_u;

  iov.iov_base = &addr;
  iov.iov_len  = sizeof(addr);

  msg.msg_name    = nullptr;
  msg.msg_namelen   = 0;
  msg.msg_iov     = &iov;
  msg.msg_iovlen    = 1;
  msg.msg_control   = cmsg_u.control;
  msg.msg_controllen  = sizeof(cmsg_u.control);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;

  do {
    socklen_t len;
    int client = accept(acceptSock, (sockaddr*) &addr, &len);
    if (client < 0) {
      continue;
    }

    *((int*) CMSG_DATA(cmsg)) = client;
    if (sendmsg(GlobalContext.dgram_pipe[Pipe::WRITE], &msg, 0) < 0) {
      close(client);
      assert(1 == 0);
    } else {
      close(client);
    }
  } while (true);
}

namespace {
void
childHangupSignal(int)
{
  LOG(ERROR) << "Child hangup signal aknowledged";
  spawnChildEventLoop();
}

void
parentHangupSignal(int)
{
  LOG(INFO) << "Parent hangup signal aknowledged";
  exit(0);
}

void
spawnChildEventLoop()
{
  LOG(INFO) << "Spawning child";
  pid_t pid = fork();

  switch (pid) {
    case -1:
      LOG(ERROR) << "Failed to fork";
      return;

    case 0:
      break;

    default:
      return;
  }

  // child -------------------------------------------------------------------
  // configure signal handlers
  if (SIG_ERR == signal(SIGCHLD, SIG_IGN)
   || SIG_ERR == signal(SIGHUP, parentHangupSignal)
   || close(GlobalContext.dgram_pipe[Pipe::WRITE]) < 0) {
    assert(1 == 0);
  }

  if (GlobalContext.config.callBack) {
    GlobalContext.config.callBack();
  }

  struct sockaddr_in  addr;
  struct cmsghdr*   cmsg;
  struct msghdr     msg;
  struct iovec    iov;
  union {
    struct cmsghdr cmsghdr;
    char       control[CMSG_SPACE(sizeof(int))];
  } cmsg_u;

  iov.iov_base = &addr;
  iov.iov_len  = sizeof(addr);

  msg.msg_name    = nullptr;
  msg.msg_namelen   = 0;
  msg.msg_iov     = &iov;
  msg.msg_iovlen    = 1;
  msg.msg_control   = cmsg_u.control;
  msg.msg_controllen  = sizeof(cmsg_u.control);

  do {
    ssize_t len = recvmsg(GlobalContext.dgram_pipe[Pipe::READ], &msg, 0);
    if (len < 0) {
      continue;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg->cmsg_level != SOL_SOCKET
     || cmsg->cmsg_type  != SCM_RIGHTS
     || cmsg->cmsg_len   != CMSG_LEN(sizeof(int))
     ) {
      continue;
    }

    int fd = *((int*) CMSG_DATA(cmsg));
    GlobalContext.master->HandleInboundSocket(fd);
  } while (true);
}
} // ns
} // ns prefork
} // ns fcgi
