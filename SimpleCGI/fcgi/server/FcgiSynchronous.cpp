#include "FcgiSynchronous.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include "SimpleCGI/fcgi/FcgiServer.hpp"
#include "SimpleCGI/common/Logging.hpp"


namespace fcgi {
namespace sync {
void
eventLoop(MasterServer* master, ServerConfig config, int socket)
{
  LOG(INFO) << "eventLoop(" << (void*)master << "...)";
  ::signal(SIGPIPE, SIG_IGN);

  if (config.callBack)
    config.callBack();

  do {
    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);

    int client = accept4(socket, (struct sockaddr*)&address,
                         &address_len, SOCK_CLOEXEC);
    if (client < 0) {
      perror(nullptr);
      continue;
    } else {
      master->HandleInboundSocket(client);
    }
  } while (true);
}

} // ns fcgi
} // ns fcgi
