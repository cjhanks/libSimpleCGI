#include "fcgi-socket.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <unistd.h>

#include "logging.hpp"

namespace fcgi {
using std::copy;
using std::string;

namespace {
void
bindAndListenSocket(int sockFD, struct sockaddr* server, int serverLen)
{
    if (::bind(sockFD, server, serverLen) < 0) {
        throw SocketCreationException("Failed to bind socket");
    }

    if (::listen(sockFD, 128) < 0) {
        throw SocketCreationException("Failed to listen");
    }
}
} // ns

int
domainSocket(const string& path)
{
    struct sockaddr_un server;
    struct stat fstat;
    if (::stat(path.c_str(), &fstat) >= 0) {
        if (unlink(path.c_str()) < 0) {
            throw SocketCreationException(path);
        }
    }

    server.sun_family = AF_UNIX;
    std::memset(server.sun_path, 0, sizeof(server.sun_path));
    copy(path.begin(), path.end(), server.sun_path);

    int sockFD = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockFD < 0) {
        throw SocketCreationException("Failed to create domain socket");
    }

    bindAndListenSocket(sockFD, (struct sockaddr*)&server, sizeof(server));

    return sockFD;
}

int
tcpSocket(const std::string& ip, int port)
{
    struct sockaddr_in server;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = ::inet_addr(ip.c_str());
    server.sin_family = AF_INET;

    int sockFD = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0) {
        throw SocketCreationException("Failed to create TCP socket");
    }

    static int True = 1;
    if (::setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(True))) {
        throw SocketCreationException("Failed to set reusable");
    }

    bindAndListenSocket(sockFD, (struct sockaddr*)&server, sizeof(server));

    return sockFD;
}

PhysicalSocket::PhysicalSocket(int sock)
    : rawSock(sock)
{}

PhysicalSocket::~PhysicalSocket()
{
    close();
}

ssize_t
PhysicalSocket::recvRaw(uint8_t* data, const size_t& size)
{
    size_t received = 0;
    do {
        ssize_t rc = ::recv(rawSock, data + received, size - received, 0);
        if (rc < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                throw SocketIOException(strerror(errno));
            }
        }
        received += rc;
    } while (received != size);

    return received;
}

ssize_t
PhysicalSocket::sendRaw(const uint8_t* data, const size_t& size)
{
    size_t sent = 0;
    do {
        ssize_t rc = ::send(rawSock, data + sent, size - sent, 0);
        if (rc < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                throw SocketIOException(strerror(errno));
            }
        } else {
            sent += rc;
        }
    } while (sent != size);
    return sent;
}

ssize_t
PhysicalSocket::sendRawVec(struct iovec* vec, const size_t count)
{
    do {
        ssize_t rc = ::writev(rawSock, vec, count);
        if (rc < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                throw SocketIOException(strerror(errno));
            }
        } else {
            return rc;
        }
    } while (true);

}

ssize_t
PhysicalSocket::recvRawVec(struct iovec* vec, const size_t count)
{
    do {
        ssize_t rc = ::readv(rawSock, vec, count);
        if (rc < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                throw SocketIOException(strerror(errno));
            }
        } else {
            return rc;
        }
    } while (true);
}

void
PhysicalSocket::close()
{
    if (rawSock < 0) {
        throw SocketStateException("Socket in wrong state");
    }

    ::close(rawSock);
    rawSock = -1;
}


LogicalSocket*
LogicalSocket::constructLogicalSocket(PhysicalSocket* physicalSocket)
{
    Header header;
    if (sizeof(header) != physicalSocket->recv(&header, 1)) {
        throw SocketIOException("Failed to receive data");
    } else {
        header.switchEndian();
    }

    if (header.isManagementRecord()) {
        assert(0 == 1);
        //return new LogicalManagementSocket(physicalSocket, header);
    } else {
        return new LogicalApplicationSocket(physicalSocket, header);
    }
}

LogicalSocket::LogicalSocket(PhysicalSocket* sock, RequestID requestId)
    : socket(sock),
      logicalRequestId(requestId),
      dataBuffer {0},
      currentWriteHead(0),
      readBuffer {0},
      currentReadHead(0),
      currentReadTail(0)
{}

LogicalSocket::~LogicalSocket()
{
    delete socket;
}

size_t
LogicalSocket::recvDataForHeader(const Header& header, uint8_t* data)
{
    ssize_t recvData = socket->recvVec(data,
                                       header.contentLength,
                                       padBuffer.data(),
                                       header.paddingLength);
    if (recvData <= 0) {
        return 0;
    }

    return recvData - header.paddingLength;
}

size_t
LogicalSocket::readData(uint8_t* data, size_t len)
{
    assert(currentReadHead <= currentReadTail);
    assert(currentReadHead < MaximumContentDataLen);
    assert(currentReadTail < MaximumContentDataLen);

    const size_t originalLen = len;

    // 1: Flush what's in current memory
    if (currentReadHead != currentReadTail && len != 0) {
        size_t currentReadWindow = currentReadTail - currentReadHead;
        if (len > currentReadWindow) {
            ::memcpy(data, &readBuffer[currentReadHead], currentReadWindow);
            currentReadHead += currentReadWindow;
            data += currentReadWindow;
            len  -= currentReadWindow;
        } else {
            ::memcpy(data, &readBuffer[currentReadHead], len);
            currentReadHead += len;
            return len;
        }
    }

    if (len == 0) {
        return 0;
    }

    assert(currentReadHead == currentReadTail);
    currentReadHead = 0;
    currentReadTail = 0;

    // 2: Access new data
    Header header(lastHeader());

    if (header.type == HeaderType::STDIN) {
        currentReadTail = recvDataForHeader(header, readBuffer.data());
        assert(currentReadTail == header.contentLength);
        header = getHeader();
        return (originalLen - len) + readData(data, len);
    } else {
        LOG(DEBUG) << "Reads are done! " << header;
    }

    return originalLen - len;
}

size_t
LogicalSocket::sendData(const uint8_t* data, size_t len)
{
    if (currentWriteHead + len >= MaximumContentDataLen) {
        size_t writeLen = MaximumContentDataLen - currentWriteHead;
        ::memcpy(dataBuffer.data() + currentWriteHead, data, writeLen);
        currentWriteHead += writeLen;
        return stdoutFlush(dataBuffer.data(), currentWriteHead)
             + sendData(data + writeLen, len - writeLen);
    } else {
        ::memcpy(dataBuffer.data() + currentWriteHead, data, len);
        currentWriteHead += len;
    }

    return 0;
}

size_t
LogicalSocket::logError(const uint8_t* data, size_t len)
{
    Header header;
    header.version       = Version::FCGI_1;
    header.type          = HeaderType::STDERR;
    header.requestId     = logicalRequestId;
    header.contentLength = static_cast<uint16_t>(len);
    header.paddingLength = static_cast<uint8_t>(0);
    header.switchEndian();

    return socket->sendVec(&header, sizeof(header), data, len);
}


void
LogicalSocket::exitCode(ProtocolStatus status)
{
    if (stdoutFlush(dataBuffer.data(), currentWriteHead)) {
        stdoutFlush(dataBuffer.data(), currentWriteHead);
    }

    MessageEndRequest endReq;
    endReq.appStatus      = 0;
    endReq.protocolStatus = status;
    endReq.switchEndian();

    Header header;
    header.version       = Version::FCGI_1,
    header.type          = HeaderType::END_REQUEST;
    header.requestId     = logicalRequestId;
    header.contentLength = static_cast<uint16_t>(sizeof(endReq));
    header.paddingLength = static_cast<uint8_t>(0);
    header.reserved      = 0;
    header.switchEndian();

    ssize_t size = socket->sendVec(&header, sizeof(header),
                                   &endReq, sizeof(endReq));
    assert(size == sizeof(header) + sizeof(endReq));
    (void) size;
}

size_t
LogicalSocket::stdoutFlush(uint8_t* data, size_t len)
{
    assert(len == currentWriteHead);
    Header header;
    header.version       = Version::FCGI_1;
    header.type          = HeaderType::STDOUT;
    header.requestId     = logicalRequestId;
    header.contentLength = static_cast<uint16_t>(len);
    header.paddingLength = static_cast<uint8_t>(0);
    header.reserved      = {0};
    header.switchEndian();

    size_t flushSize = socket->sendVec(&header, sizeof(header), data, len);
    currentWriteHead -= len;
    assert(currentWriteHead == 0);

    return flushSize;
}

size_t
LogicalSocket::stderrFlush()
{
    return logError(nullptr, 0);
}

Header
LogicalSocket::getHeader()
{
    Header header;
    if (!socket->recv(&header, 1)) {
        throw SocketIOException("Failed to receive");
    } else {
        header.switchEndian();
    }
    logicalLastHeader = header;
    return header;
}

LogicalApplicationSocket::LogicalApplicationSocket(
        PhysicalSocket* physicalSocket, Header header)
    : LogicalSocket(physicalSocket, header.requestId)
{
    if (sizeof(beginRequest) != recvDataForHeader(header, &beginRequest)) {
        assert(1 == 0);
    } else {
        beginRequest.switchEndian();
    }
}

bool
LogicalApplicationSocket::mergeKeyValueMap(Header& header, KeyValueMap& kvMap)
{
    auto contentLength = header.contentLength;
    assert(contentLength <= dataBuffer.size());
    if (contentLength != recvDataForHeader(header, dataBuffer.data())) {
        return false;
    }

    return readIntoKeyValueMap(dataBuffer.data(), header.contentLength, kvMap);
}


} // ns fcgi
