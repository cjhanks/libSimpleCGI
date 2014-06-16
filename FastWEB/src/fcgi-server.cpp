
#include "fcgi-server.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <memory>

#include "fcgi-socket.hpp"
#include "fcgi-handler.hpp"
#include "logging.hpp"

namespace fcgi {
using std::unique_ptr;

////////////////////////////////////////////////////////////////////////////////
namespace synchronous {
void
eventLoop(MasterServer* master, ServerConfig config, int socket)
{
    ::signal(SIGPIPE, SIG_IGN);
    
    do {
        struct sockaddr_in address;
        socklen_t          address_len;
        int client = accept4(socket, (struct sockaddr*)&address, 
                             &address_len, SOCK_CLOEXEC);
        if (client < 0) {
            continue;
        }
       
        unique_ptr<LogicalSocket> sock(
            LogicalSocket::constructLogicalSocket(new PhysicalSocket(client)));
        applicationHandler(master,
                           static_cast<LogicalApplicationSocket*>(sock.get()));
    } while (true);
}
} // ns synchronous 

////////////////////////////////////////////////////////////////////////////////

namespace prefork {
void
eventLoop(MasterServer* master, ServerConfig config, int socket)
{
    
}
} // ns prefork

////////////////////////////////////////////////////////////////////////////////

MasterServer::MasterServer(ServerConfig config, int socket)
    : serverConfig(config), rawSock(socket)
{  
    LOG(INFO) << "MasterServer(..., " << socket << ")";
    int       optVal;
    socklen_t optLen = sizeof(optVal);
    if (getsockopt(socket, SOL_SOCKET, SO_ACCEPTCONN, &optVal, &optLen)
            < 0) {
        throw MasterServerException("Failed to get accept status");
    }
    if (0 == optVal) {
        throw MasterServerException("Socket is not in accept() mode");
    }
}

void
MasterServer::serveForever() 
{
    switch (serverConfig.concurrencyModel) {
        case ServerConfig::ConcurrencyModel::NONE:
            synchronous::eventLoop(this, serverConfig, rawSock);
            break;
        case ServerConfig::ConcurrencyModel::THREAD:
            break;
        case ServerConfig::ConcurrencyModel::PROCESS:
            break;
    }
}
} // ns fcgi
