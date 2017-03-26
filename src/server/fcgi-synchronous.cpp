#include "server/fcgi-synchronous.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include "fcgi-server.hpp"
#include "logging.hpp"


namespace fcgi {
namespace sync {
void
eventLoop(MasterServer* master, ServerConfig, int socket)
{
    LOG(INFO) << "eventLoop(" << (void*)master << "...)";
    ::signal(SIGPIPE, SIG_IGN);

    do {
        struct sockaddr_in address;
        socklen_t          address_len;
        int client = accept4(socket, (struct sockaddr*)&address,
                             &address_len, SOCK_CLOEXEC);
        LOG(DEBUG) << "Client = " << client;
        if (client < 0) {
            continue;
        } else {
            master->handleInboundSocket(client);
        }
    } while (true);
}

} // ns fcgi
} // ns fcgi
