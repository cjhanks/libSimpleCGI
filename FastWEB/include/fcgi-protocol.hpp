#ifndef __FCGI_PROTOCOL_HPP
#define __FCGI_PROTOCOL_HPP

#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <type_traits>


namespace fcgi {
static constexpr size_t MaximumContentDataLen = (1 << 16);
static constexpr size_t MaximumPaddingDataLen = (1 << 8 );

using KeyValueMap = std::map<std::string, std::string>;

enum class Version : std::uint8_t {
    FCGI_1     = 1
};

enum class RequestClass {
    UNDEFINED   = 0,
    MANAGEMENT  = 1,
    APPLICATION = 2
};

enum class RequestRole : std::uint16_t {
    UNDEFINED  = 0,
    RESPONDER  = 1,
    AUTHORIZER = 2,
    FILTER     = 3
};

enum class HeaderType : std::uint8_t {
    BEGIN_REQUEST       = 1,
    ABORT_REQUEST       = 2,
    END_REQUEST         = 3,
    PARAMS              = 4,
    STDIN               = 5,
    STDOUT              = 6,
    STDERR              = 7,
    DATA                = 8,
    GET_VALUES          = 9,
    GET_VALUES_RESULT   = 10,
    UNKNOWN_TYPE        = 11,
    MAXTYPE             = UNKNOWN_TYPE
};

inline std::string
headerTypeToString(HeaderType t) {
    switch (t) {
        case HeaderType::BEGIN_REQUEST:
            return "BEGIN_REQUEST";
        case HeaderType::ABORT_REQUEST:
            return "ABORT_REQUEST";
        case HeaderType::END_REQUEST:
            return "END_REQUEST";
        case HeaderType::PARAMS:
            return "PARAMS";
        case HeaderType::STDIN:
            return "STDIN";
        case HeaderType::STDOUT:
            return "STDOUT";
        case HeaderType::STDERR:
            return "STDERR";
        case HeaderType::DATA:
            return "DATA";
        case HeaderType::GET_VALUES:
            return "GET_VALUES";
        case HeaderType::GET_VALUES_RESULT:
            return "GET_VALUES_RESULT";
        case HeaderType::UNKNOWN_TYPE:
            return "UNKNOWN_TYPE";
    }
};

using RequestID = std::uint16_t;

struct Header {
    Version         version;
    HeaderType      type;
    RequestID       requestId;
    std::uint16_t   contentLength;
    std::uint8_t    paddingLength;
    std::uint8_t    reserved;

    inline bool
    isManagementRecord()
    {
        return 0 == requestId;
    }

    inline void
    switchEndian()
    {
        requestId     = __builtin_bswap16(requestId);
        contentLength = __builtin_bswap16(contentLength);
    }

    friend std::ostream& operator<<(std::ostream&, const Header&);
} __attribute__((packed));

inline std::ostream&
operator<<(std::ostream& strm, const Header& h)
{
    strm << "Header:"
         << (size_t) h.version
         << "->"
         << headerTypeToString(h.type)
         << ":@"
         << (size_t) h.requestId
         << ":["
         << (size_t) h.contentLength
         << "]:["
         << (size_t) h.paddingLength
         << "]";
    return strm;
}

static constexpr std::size_t HeaderLen = sizeof(Header);
static_assert(HeaderLen == 8
            , "Incorrectly defined header");

bool
readIntoKeyValueMap(const std::uint8_t* data, size_t len,
                    KeyValueMap& keyValueMap);

////////////////////////////////////////////////////////////////////////////////
// MESSAGES
struct MessageBeginRequest {
    RequestRole     role;
    std::uint8_t    flags;
    std::uint8_t    reserved[5];

    inline void
    switchEndian()
    {
        std::uint16_t r = static_cast<std::uint16_t>(role);
        r = __builtin_bswap16(r);
        role = static_cast<RequestRole>(r);
    }

    inline bool
    shouldKeepConnection() const
    {
        static constexpr size_t KeepConn = 1;
        return flags & KeepConn;
    }

    friend std::ostream& operator<<(std::ostream&, const MessageBeginRequest&);
} __attribute__((packed));

inline std::ostream&
operator<<(std::ostream& strm, const MessageBeginRequest& msg)
{
    strm << "Begin:"
         << (size_t) msg.role
         << ":Keep?"
         << msg.shouldKeepConnection();
}

enum class ProtocolStatus : std::uint8_t {
    REQUEST_COMPLETE    = 1,
    CANT_MPX_CONN       = 2,
    OVERLOADED          = 3,
    UNKNOWN_ROLE        = 4
};

struct MessageEndRequest {
    std::uint32_t       appStatus;
    ProtocolStatus      protocolStatus;
    std::uint8_t        reserved[3];

    inline void
    switchEndian()
    {
        appStatus = __builtin_bswap32(appStatus);
    }

} __attribute__((packed));

struct MessageUnknown {
} __attribute__((packed));

union Message {
    MessageBeginRequest begin;
    MessageEndRequest end;
    MessageUnknown unknown;
};

static_assert(sizeof(MessageEndRequest) == 8
            , "Invalid End request size");

} // ns fcgi

#endif //__FCGI_PROTOCOL_HPP
