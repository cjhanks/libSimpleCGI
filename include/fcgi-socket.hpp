#ifndef __FCGI_SOCKET_HPP
#define __FCGI_SOCKET_HPP

#include <sys/uio.h>
#include <array>
#include <stdexcept>

#include "fcgi-protocol.hpp"
#include "logging.hpp"


namespace fcgi {
class SocketCreationException : public std::runtime_error {
public:
    SocketCreationException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

class SocketStateException : public std::runtime_error {
public:
    SocketStateException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

class SocketIOException : public std::runtime_error {
public:
    SocketIOException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};


/**
 */
int
domainSocket(const std::string& path);

int
tcpSocket(const std::string& ip, int port);

class PhysicalSocket {
public:
    PhysicalSocket(int sock);
    ~PhysicalSocket();

    PhysicalSocket(PhysicalSocket&) = delete;
    PhysicalSocket&
    operator=(PhysicalSocket&) = delete;

    template <typename _T>
    ssize_t
    recv(_T* data, const size_t& len) {
        return recvRaw((std::uint8_t*) data, len * sizeof(_T));
    }

    template <typename... _Args>
    ssize_t
    recvVec(_Args&&... args) {
        static constexpr size_t ArgCount = sizeof...(_Args) / 2;
        struct iovec vec[ArgCount];
        buildRawVec(vec, args...);
        return recvRawVec(vec, ArgCount);
    }


    template <typename _T>
    ssize_t
    send(const _T* data, const size_t& len) {
        return sendRaw(static_cast<const std::uint8_t*>(data),
                       len * sizeof(_T));
    }

    template <typename... _Args>
    ssize_t
    sendVec(_Args&&... args) {
        static constexpr size_t ArgCount = sizeof...(_Args) / 2;
        struct iovec vec[ArgCount] = {};
        buildRawVec(vec, std::forward<_Args>(args)...);
        return sendRawVec(vec, ArgCount);
    }

    void
    close();

private:
    int rawSock;

    ssize_t
    recvRaw(std::uint8_t* data, const size_t& size);

    ssize_t
    sendRaw(const std::uint8_t* data, const size_t& size);

    template <typename _Data, typename... _Args>
    void
    buildRawVec(struct iovec* vec, _Data data, const size_t len,
                _Args&&...args) {
        vec->iov_base = (void*)data;
        vec->iov_len  = len;
        buildRawVec(vec + 1, args...);
    }

    template <typename _Data>
    void
    buildRawVec(struct iovec* vec, _Data data, const size_t len) {
        vec->iov_base = (void*)data;
        vec->iov_len  = len;
    }

    ssize_t
    sendRawVec(struct iovec* vec, const size_t count);

    ssize_t
    recvRawVec(struct iovec* vec, const size_t count);
};

class LogicalSocket {
public:
    static LogicalSocket*
    constructLogicalSocket(PhysicalSocket* physicalSocket);

    virtual ~LogicalSocket();

    RequestID
    requestId() const;

    virtual RequestClass
    requestClass() const = 0;

    size_t
    readData(std::uint8_t* data, size_t len);

    size_t
    sendData(const std::uint8_t* data, size_t len);

    size_t
    logError(const std::uint8_t* data, size_t len);

    void
    exitCode(ProtocolStatus status);

    Header
    getHeader();

    Header
    lastHeader() const { return logicalLastHeader; }

protected:
    LogicalSocket(PhysicalSocket* sock, RequestID requestId);

    template <typename _Tp>
    size_t
    recvDataForHeader(const Header& header, _Tp* data) {
        return recvDataForHeader(header,
                                 reinterpret_cast<std::uint8_t*>(data));
    }

    size_t
    recvDataForHeader(const Header& header, std::uint8_t* data);

    size_t
    stdoutFlush(std::uint8_t* data, size_t len);

    size_t
    stderrFlush();

    PhysicalSocket* socket;
    RequestID    logicalRequestId;
    Header logicalLastHeader;

    std::array<std::uint8_t, MaximumContentDataLen> dataBuffer;
    size_t currentWriteHead;

    std::array<std::uint8_t, MaximumContentDataLen> readBuffer;
    size_t currentReadHead;
    size_t currentReadTail;

private:
    std::array<std::uint8_t, MaximumPaddingDataLen> padBuffer;
};

class LogicalApplicationSocket : public LogicalSocket {
public:
    LogicalApplicationSocket(PhysicalSocket* physicalSocket,
                             Header header);
    virtual ~LogicalApplicationSocket() = default;

    virtual RequestClass
    requestClass() const final { return RequestClass::APPLICATION; }

    bool
    mergeKeyValueMap(Header& header, KeyValueMap& kvMap);

private:
    MessageBeginRequest beginRequest;

};

class LogicalManagementSocket : public LogicalSocket {
public:
    LogicalManagementSocket(PhysicalSocket* physicalSocket,
                            Header header);
    virtual ~LogicalManagementSocket() = default;

    virtual RequestClass
    requestClass() const final { return RequestClass::MANAGEMENT; }

private:
    KeyValueMap keyValParams;
};

} // ns fcgi

#endif //__FCGI_SOCKET_HPP
