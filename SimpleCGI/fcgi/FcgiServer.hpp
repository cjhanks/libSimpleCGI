#ifndef __FCGI_SERVER_HPP
#define __FCGI_SERVER_HPP

#include <functional>
#include <string>
#include <stdexcept>

#include "FcgiHttp.hpp"
#include "FcgiHandler.hpp"
#include "FcgiIo.hpp"

namespace fcgi {
using ServeCallback = std::function<void()>;
using ServeFallback = std::function<bool(HttpRequest&, HttpResponse&)>;


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
  ServeFallback catchAll;

  ServerConfig()
    : concurrencyModel(ConcurrencyModel::SYNCHRONOUS), childCount(1)
  {}

  void
  DumpTo(std::ostream&) const;
};

class MasterServer {
public:
  MasterServer(ServerConfig config, int socket);

  void
  ServeForever();

  void
  HandleInboundSocket(int sock);

  void
  DumpTo(std::ostream&) const;

  fcgi::Assets&
  Assets() { return serverAssets; }

  MatchingRoot&
  Routes() { return httpRoutes; }

private:
  const ServerConfig serverConfig;
  fcgi::Assets serverAssets;
  MatchingRoot httpRoutes;
  int rawSock;

  void
  applicationHandler(LogicalApplicationSocket* client);

  void
  ImplHandleInboundSocket(int sock);

};
} // ns fcgi

#endif //__FCGI_SERVER_HPP
