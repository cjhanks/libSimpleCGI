#ifndef __SERVER_THREADED_HPP_
#define __SERVER_THREADED_HPP_

namespace fcgi {
class MasterServer;
class ServerConfig;

namespace threaded {
void
eventLoop(MasterServer* master, ServerConfig config, int socket);
} // ns threaded
} // ns fcgi

#endif //__SERVER_THREADED_HPP_
