
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
#include "FcgiHandler.hpp"
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
    if (!maybeRoute) {
      if (serverConfig.catchAll)
        serverConfig.catchAll(req, res);
      else
        res.SetResponse(HttpHeader(404, "text/html"));
    } else {
      maybeRoute.call(req, res);
    }
  } catch (...) {}
}

void
MasterServer::DumpTo(ostream& strm) const
{
  using std::endl;
  strm << string(80, '=') << endl
     << "FASTWEB: " << VersionToString(CurrentVersion) << endl
     << string(80, '-') << endl;

  serverConfig.DumpTo(strm);

  strm << httpRoutes;
  serverAssets.DumpTo(strm);
}


void
ServerConfig::DumpTo(ostream& strm) const
{
  auto concurrencyModelStr = [&]() {
    switch (concurrencyModel) {
      case ConcurrencyModel::SYNCHRONOUS:
        return "SYNCHRONOUS";
      case ConcurrencyModel::THREADED:
        return "THREADED";
      case ConcurrencyModel::PREFORKED:
        return "PREFORKED";
    }

    return "UNDEFINED";
  };

  strm << "Concurrency Model: " << concurrencyModelStr() << " -> ";
  if (concurrencyModel == ConcurrencyModel::SYNCHRONOUS) {
    strm << "1";
  } else {
    strm << childCount;
  }
  strm << std::endl;
}
} // ns fcgi
