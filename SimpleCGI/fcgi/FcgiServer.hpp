#ifndef __FCGI_SERVER_HPP
#define __FCGI_SERVER_HPP

#include <functional>
#include <string>
#include <stdexcept>

#include "FcgiHttp.hpp"
#include "FcgiHandler.hpp"
#include "FcgiIo.hpp"
#include "FcgiRequest.hpp"
#include "SimpleCGI/common/Logging.hpp"

namespace fcgi {
/// {
class HttpRequest;
class HttpResponse;
/// }

/// @{
/// Call back typedefs.
using ServeCallback = std::function<void()>;
using ServeFallback = Route;
/// @}


/// @class MasterServerException
///
/// This exception is thrown if the underlying socket is improperly
/// configured for accepting connections.
class MasterServerException : public std::runtime_error {
public:
  MasterServerException(const std::string& msg)
    : std::runtime_error(msg)
  {}
};

/// @struct ServerConfig
///
/// Structure used for configuring how the MasterServer is going to
/// behave.
///
struct ServerConfig {
  ///  @enum ConcurrencyModel
  ///
  ///  SYNCHRONOUS:
  ///    Only a single request is handled at a time, and it is
  ///    handled in the same thread that
  ///    fcgi::MasterServer::ServeForever()` is called in.
  ///
  ///    When set:
  ///      `config.childCount` is meaningless.
  ///
  ///  THREADED:
  ///    Creates a pool of reusable `config.childCount` threads. Note that
  ///    there is always `config.childCount + 1` threads.  This is because
  ///    one thread is exclusively responsible for `accept` calls on the
  ///    socket and may be used in various background tasks.
  ///
  ///    Threads are *not* detached from the main process.  Abort calls in
  ///    any thread will terminate all threads.
  ///
  ///    Important:
  ///      Child sockets are passed through a thread-safe blocking queue.
  ///      This queue can theoretically overflow, it is expected that
  ///      queuing is performed in the downstream FastCGI provider.
  ///
  ///  PREFORKED:
  ///    There are `config.childCount + 1` processes.  The main process
  ///    is responsible for accepting sockets and sending them to children
  ///    in different processes.
  ///
  ///    Crashes in the main thread will cause an abort of all children,
  ///    connections will not be drained.
  ///
  ///    Crashes in child threads will be restarted by main thread.
  enum class ConcurrencyModel {
    SYNCHRONOUS,
    THREADED,
    PREFORKED,
  };

  ConcurrencyModel concurrencyModel;

  /// Number of children which will be spawned.
  size_t childCount;

  /// Called before a spawned worker is to accept its first request.
  ServeCallback callBack;

  /// Called if no routes match an HTTP request.
  ServeFallback catchAll;

  ServerConfig()
    : concurrencyModel(ConcurrencyModel::SYNCHRONOUS)
    , childCount(1)
  {}

  void
  DumpTo(std::ostream&) const;
};

class MasterServer {
public:
  MasterServer(ServerConfig config, int socket);

  /// Enter the event loop and begin serving until signals are raised
  /// against the process.
  int
  ServeForever();

  /// {
  /// Installs a route to be handled by the server.
  ///
  /// routeStr should be something like -
  ///   "/route/<arg1>/<arg2>/value"
  ///
  /// route is a function callback.
  ///
  /// verbSet is an optional set of HTTP verbs to be handled,
  /// by default it will be all verbs.
  void
  InstallRoute(const std::string& routeStr, const Route& route)
  { httpRoutes.InstallRoute(routeStr, route); }

  void
  InstallRoute(const std::string& routeStr, const Route& route,
               const VerbSet& verbSet)
  { httpRoutes.InstallRoute(routeStr, route, verbSet); }
  /// }


  /// This method is incidentally exposed due to design issues
  /// in the event loops.  It should not be used unless you are
  /// implementing your own event loop.
  void
  HandleInboundSocket(int sock);

  void
  DumpTo(std::ostream&) const;

private:
  const ServerConfig serverConfig;
  fcgi::Assets serverAssets;
  MatchingRoot httpRoutes;
  int rawSock;

  void
  applicationHandler(LogicalApplicationSocket* client);

  void
  ImplHandleInboundSocket(int sock);

  // {
  // Methods needed by reqests and responses.
  friend class HttpRequest;
  friend class HttpResponse;
  fcgi::Assets&
  Assets() { return serverAssets; }

  MatchingRoot&
  Routes() { return httpRoutes; }
  // }
};
} // ns fcgi

#endif //__FCGI_SERVER_HPP
