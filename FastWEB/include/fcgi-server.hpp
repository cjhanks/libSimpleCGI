#ifndef __FCGI_SERVER_HPP
#define __FCGI_SERVER_HPP

#include <string>
#include <stdexcept>

#include "fcgi-http.hpp"
#include "fcgi-handler.hpp"

namespace fcgi {
class MasterServerException : public std::runtime_error {
public:
    MasterServerException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

struct ServerConfig {
    enum class ConcurrencyModel {
        NONE,
        THREAD,
        PROCESS
    };

    ServerConfig()
        : concurrencyModel(ConcurrencyModel::NONE), childCount(1)
    {}

    ConcurrencyModel concurrencyModel;
    size_t childCount;
};

class MasterServer {
public:
    MatchingRoot HttpRoutes;

    MasterServer(ServerConfig config, int socket);
    
    void
    serveForever();

private:
    const ServerConfig serverConfig;
    int rawSock;

};
} // ns fcgi

#endif //__FCGI_SERVER_HPP
