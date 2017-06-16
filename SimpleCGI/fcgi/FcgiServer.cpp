#include "FcgiServer.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <memory>

#include "Fcgi.hpp"
#include "FcgiSocket.hpp"
#include "FcgiRequest.hpp"
#include "FcgiResponse.hpp"
#include "server/FcgiPreFork.hpp"
#include "server/FcgiSynchronous.hpp"
#include "server/FcgiThreaded.hpp"

#include "SimpleCGI/common/Logging.hpp"

namespace fcgi {
using std::unique_ptr;
using std::ostream;
using std::string;


MasterServer::MasterServer(ServerConfig config, int socket)
  : serverConfig(config), rawSock(socket)
{
  int     optVal;
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
MasterServer::InstallRoute(
    const std::string& routeStr, const Route& route)
{
  InstallRoute(routeStr, route, {HttpVerb::ANY});
}

void
MasterServer::InstallRoute(
    const std::string& routeStr, const Route& route,
    const VerbSet& verbSet)
{
  httpRoutes.InstallRoute(
      routeStr,
      InstalledRoute({route}, verbSet)
  );
}

int
MasterServer::ServeForever()
{
  switch (serverConfig.concurrencyModel) {
    case ServerConfig::ConcurrencyModel::SYNCHRONOUS:
      sync::eventLoop(this, serverConfig, rawSock);
      break;
    case ServerConfig::ConcurrencyModel::THREADED:
      threaded::eventLoop(this, serverConfig, rawSock);
      break;
    case ServerConfig::ConcurrencyModel::PREFORKED:
      prefork::eventLoop(this, serverConfig, rawSock);
      break;
  }

  // FIXME
  return 0;
}

void
MasterServer::HandleInboundSocket(int sock)
{
  if (sock < 0) {
    LOG(ERROR)
        << "Invalid socket provided for handling";
    return;
  }

  try {
    ImplHandleInboundSocket(sock);
  } catch (const fcgi::SocketIOException& e) {
    LOG(INFO) << "Caught IO Exception";
  } catch (const std::runtime_error& e) {
    LOG(INFO) << "Caught IO Exception";
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
  } catch (...) {
    LOG(ERROR) << "UNKNOWN EXCEPTION CAUGHT";
  }
}

void
MasterServer::ImplHandleInboundSocket(int sock)
{
  unique_ptr<LogicalSocket> logic(
    LogicalSocket::ConstructLogicalSocket(new PhysicalSocket(sock)));

  switch (logic->requestClass()) {
    case RequestClass::APPLICATION:
      applicationHandler(
            static_cast<LogicalApplicationSocket*>(logic.get()));
      break;

    case RequestClass::MANAGEMENT:
      LOG(ERROR) << "Received unhandled Management Request";
      close(sock);
      break;

    default:
      LOG::CHECK(0 == 1)
          << "Unknown request type from FCGI host";
      close(sock);
      break;
  }
}

void
MasterServer::applicationHandler(LogicalApplicationSocket* client)
{
  try {
    HttpRequest req(client);
    HttpResponse res(client);

    auto maybeRoute = req.GetRoute(this);
    if (maybeRoute) {
      maybeRoute.Call(req, res);
    } else {
      if (serverConfig.catchAll)
        serverConfig.catchAll(req, res);
      else
        res.SetResponse(HttpHeader(404, "text/html"));
    }
  } catch (...) {}
}
} // ns fcgi
