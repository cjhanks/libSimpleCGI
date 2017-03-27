#ifndef __SERVER_SYNCHRONOUS_HPP_
#define __SERVER_SYNCHRONOUS_HPP_

namespace fcgi {
class MasterServer;
struct ServerConfig;

namespace sync {
void
eventLoop(MasterServer* master, ServerConfig config, int socket);
} // ns sync
} // ns fcgi

#endif //__SERVER_SYNCHRONOUS_HPP_
