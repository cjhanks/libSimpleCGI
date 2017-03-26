#ifndef __SERVER_PREFORK_HPP_
#define __SERVER_PREFORK_HPP_

namespace fcgi {
class MasterServer;
class ServerConfig;

namespace prefork {
void
eventLoop(MasterServer* master, ServerConfig config, int socket);
} // ns prefork
} // ns fcgi

#endif //__SERVER_PREFORK_HPP_
