#ifndef __FCGI_SERVER_HPP
#define __FCGI_SERVER_HPP

#include <string>
#include <stdexcept>

#include "fcgi-http.hpp"
#include "fcgi-handler.hpp"
#include "fcgi-io.hpp"

namespace fcgi {
using ServeCallback = std::function<void()>;

class MasterServerException : public std::runtime_error {
public:
    MasterServerException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

struct ServerConfig {
    enum class ConcurrencyModel {
        SYNCHRONOUS,
        THREADED,
        PREFORKED, 
    };
    
    ConcurrencyModel concurrencyModel;
    size_t childCount;
    ServeCallback callBack;

    ServerConfig()
        : concurrencyModel(ConcurrencyModel::SYNCHRONOUS), childCount(1)
    {}
    
    void
    dumpTo(std::ostream&) const;
};

class MasterServer {
public:
    MasterServer(ServerConfig config, int socket);
    
    void
    serveForever();

    void
    handleInboundSocket(int sock);

    void
    dumpTo(std::ostream&) const;

    Assets&
    assets() { return serverAssets; }
    
    MatchingRoot&
    routes() { return httpRoutes; }

private:
    const ServerConfig serverConfig;
    Assets serverAssets;
    MatchingRoot httpRoutes;
    int rawSock;

};
} // ns fcgi

#endif //__FCGI_SERVER_HPP
