#include "fastcgi-application.hpp"

#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <sstream>

#include "fcgi.hpp"
#include "logging.hpp"
#include "exceptions.hpp"
#include "fcgi-responder.hpp"
#include "network-parsing.hpp"

namespace fcgi {
using std::begin;
using std::end;
using std::copy;
using std::fill;
using std::string;
using std::stringstream;


static constexpr const char* WebServerAddressEnvKey = "FCGI_WEB_SERVER_ADDRS";

DomainSocket::DomainSocket(const string& hostPath)
    : hostPath(hostPath), sockFD(-1)
{
    struct sockaddr_un server;
    struct stat fstat;
    if (::stat(hostPath.c_str(), &fstat) >= 0) {
        if (unlink(hostPath.c_str()) < 0) {
            throw DomainPermissionIssue(hostPath);
        }
    }
    
    server.sun_family = AF_UNIX;
    std::memset(server.sun_path, 0, sizeof(server.sun_path));
    copy(hostPath.begin(), hostPath.end(), server.sun_path);

    sockFD = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockFD < 0) {
        throw SocketCreationException("Failed to create domain socket");
    }

    if (::bind(sockFD, (struct sockaddr*) &server, sizeof(server)) < 0) {
        throw SocketCreationException("Failed to bind socket");
    } 

    if (::listen(sockFD, 32) < 0) {
        throw SocketCreationException("Failed to listen");
    }
}

DomainSocket::~DomainSocket()
{
    if (sockFD > 0) {
        ::close(sockFD);
    }
}


DomainSocket::operator int()
{
    return dup(sockFD);
}

KeyValueMap
FastCGI::defaultGetValueMap = {
    {MaximumConnections    , "1"},
    {MaximumRequests       , "1"},
    {MultiplexesConnections, "0"}
};

FastCGI::FastCGI(int sock)
    : serverSocket(sock)
{
    // if the webserver address' are define, insert them
    static constexpr char splitToken = ',';
    string buffer;
    string serverAddressString(getenv(WebServerAddressEnvKey) ?
                               getenv(WebServerAddressEnvKey) :
                               string(""));
    stringstream serverAddressStream(serverAddressString);

    while (getline(serverAddressStream, buffer, splitToken)) {
        static string trimmedCharacters(" \f\n\r\t\v");
        buffer.erase(buffer.find_last_not_of(trimmedCharacters) + 1
                   , buffer.find_first_not_of(trimmedCharacters));
        if (buffer.size()) {
            serverAddresses.insert(buffer);
        }
        LOG(INFO) << "hostIP:" << buffer;
    }
}
    
FastCGI::~FastCGI() {
    ::close(serverSocket);
}

void
FastCGI::enterRequestLoop()
{
    do {
        struct sockaddr_in address;
        socklen_t address_len;
        
        UniqueSocket client(accept(serverSocket, (struct sockaddr*) &address, 
                            &address_len));
        if (!client) {
            throw NullClientException("Failed to accept client");
        }

        // if server addresses are defined, ensure that the client is a member of
        // that set.
        if (serverAddresses.size()) {
            const string clientIpString(inet_ntoa(address.sin_addr));
            LOG(DEBUG) << "clientIP:" << clientIpString;
            if (0 == serverAddresses.count(clientIpString)) {
                throw UnknownClientException(string("Unknown client: ")
                                           + clientIpString);
            }
        } 
        
        // receive the header 
        Header header(client.readHeader());
        if (header.isManagementRecord()) {
            handleManagementRecord(header, client);
        } else {
            handleApplicationRequest(header, client);
        }
    } while (true);
}

void
FastCGI::handleManagementRecord(const Header& header, UniqueSocket& socket)
{
    LOG(DEBUG) << "received management header";
    if (header.type != HeaderType::GET_VALUES) {
        throw ClientProtocolException(
                "received unexpected header:"
               + std::to_string(static_cast<int>(header.type)));
    }
    VectorBuffer buffer;

    // receive the request elements 
    socket.readIntoBuffer(header, buffer);
    KeyValueMap keyValueReq;
    readBufferIntoKeyValuePair(buffer, keyValueReq);
    
    // build the response map 
    KeyValueMap keyValueRes;
    for (auto& req: keyValueReq) {
        auto res = defaultGetValueMap.find(req.first);
        if (res == defaultGetValueMap.end()) {
            throw InvalidManagementQuery("bad query: " + req.first);
        } else {
            keyValueRes[req.first] = res->second;
        }
    }

    saveKeyValuePairIntoBuffer(keyValueRes, buffer);
}

void
FastCGI::handleApplicationRequest(const Header& header, UniqueSocket& socket)
{
    VectorBuffer buffer;
    socket.readIntoBuffer(header, buffer);
    if (header.contentLength != buffer.size) {
        throw ClientProtocolException("Invalid buffer size");
    }

    RawBegin rawBeginMessage;
    std::memcpy(&rawBeginMessage, buffer.contentBuffer.data(), 
                header.contentLength);
    Begin beginMessage = rawBeginToNativeBegin(rawBeginMessage);

    switch (beginMessage.role) {
        case Begin::Role::RESPONDER:
            FCGI_Responder(header, beginMessage)(socket, buffer);
            break;

        case Begin::Role::AUTHORIZER:
            break;

        case Begin::Role::FILTER:
            break;
    }
}
} //ns fcgi