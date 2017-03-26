
#include "fcgi-server.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <memory>

#include "fcgi.hpp"
#include "fcgi-socket.hpp"
#include "fcgi-handler.hpp"
#include "server/fcgi-pre-fork.hpp"
#include "server/fcgi-synchronous.hpp"
#include "logging.hpp"

namespace fcgi {
using std::unique_ptr;
using std::ostream;
using std::string;

namespace {
void
application(MasterServer* master, LogicalApplicationSocket* sock)
{
    try {
        applicationHandler(master, sock);
    } catch (const SocketStateException& e) {
        LOG(INFO) << "E1: " << e.what();
    } catch (const SocketIOException& e) {
        LOG(INFO) << "E2: " << e.what();
    } catch (...) {
    }
}
} // ns


MasterServer::MasterServer(ServerConfig config, int socket)
    : serverConfig(config), rawSock(socket)
{  
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
        case ServerConfig::ConcurrencyModel::SYNCHRONOUS:
            sync::eventLoop(this, serverConfig, rawSock);
            break;
        case ServerConfig::ConcurrencyModel::THREADED:
            break;
        case ServerConfig::ConcurrencyModel::PREFORKED:
            LOG(INFO) << "Server forked";
            prefork::eventLoop(this, serverConfig, rawSock);
            break;
    }
}

void
MasterServer::handleInboundSocket(int sock)
{
    unique_ptr<LogicalSocket> logic(
        LogicalSocket::constructLogicalSocket(new PhysicalSocket(sock)));

    switch (logic->requestClass()) {
        case RequestClass::APPLICATION:
            application(this, 
                        static_cast<LogicalApplicationSocket*>(logic.get()));
            break;

        case RequestClass::MANAGEMENT:
            break;

        default:
            assert(1 == 0);
            close(sock);
            break;
    }
}

void
MasterServer::dumpTo(ostream& strm) const
{
    using std::endl;
    strm << string(80, '=') << endl
         << "FASTWEB: " << versionToString(CurrentVersion) << endl
         << string(80, '-') << endl;

    serverConfig.dumpTo(strm);
    
    strm << httpRoutes;
    serverAssets.dumpTo(strm);
}
    
void
ServerConfig::dumpTo(ostream& strm) const
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
