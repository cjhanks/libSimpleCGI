#ifndef __FCGI_HPP
#define __FCGI_HPP
/**
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 */

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>

namespace fcgi {

static constexpr int ListenSocket = 0;
static constexpr size_t MaximumContentDataLen = (1 << 16) - 1;
static constexpr size_t MaximumPaddingDataLen = (1 << 8 ) - 1;

struct VectorBuffer {
    std::array<std::uint8_t, MaximumContentDataLen> contentBuffer;
    std::array<std::uint8_t, MaximumPaddingDataLen> paddingBuffer;
    size_t size;
};

using KeyValueMap = std::map<std::string, std::string>;

enum class Version : std::uint8_t {
    FCGI_1     = 1
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

    friend std::ostream& operator<<(std::ostream&, Header&);
} __attribute__((packed));

inline std::ostream& 
operator<<(std::ostream& strm, Header& header)
{
    strm << std::endl
         << "Header:"
         << std::endl
         << "  version:    " << (int)header.version
         << std::endl
         << "  type:       " << (int)header.type
         << std::endl
         << "  requestId:  " << (int)header.requestId 
         << std::endl
         << "  contentLen: " << (int)header.contentLength
         << std::endl
         << "  paddingLen: " << (int)header.paddingLength
         << std::endl;
    return strm;
}

static constexpr std::size_t HeaderLen = sizeof(Header);
static_assert(HeaderLen == 8
            , "Incorrectly defined header");

struct RawBegin {
    enum class Role : std::uint16_t {
        RESPONDER   = 1,
        AUTHORIZER  = 2,
        FILTER      = 3
    };
    
    Role            role;
    std::uint8_t    flags;
    std::uint8_t    reserved[5];
    
    inline bool
    keepConnection() const
    {
        static constexpr size_t Mask = 1;
        return this->flags & Mask;
    }
};

using Begin = RawBegin;

inline Begin
rawBeginToNativeBegin(const RawBegin& header)
{
    Begin::Role correct = static_cast<Begin::Role>(
                    __builtin_bswap16(static_cast<std::uint16_t>(header.role)));
    return (Begin) {
        .role     = correct,
        .flags    = header.flags,
    };
}

/*
struct EndRequest {
    enum Protocol {
        REQUEST_COMPLETE    = 0,
        CANT_MPX_CONN       = 1,
        OVERLOADED          = 2,
        UNKNOWN_ROLE        = 3
    };

    std::uint32_t   appStatus;
    Protocol        protocolStatus:8;
    std::uint8_t    reserved[3];
};
*/

static constexpr const char* MaximumConnections     = "FCGI_MAX_CONNS";
static constexpr const char* MaximumRequests        = "FCGI_MAX_REQS";
static constexpr const char* MultiplexesConnections = "FCGI_MPXS_CONNS";
struct ManagementLiteral {
    enum Values { 
        MaxConns, 
        MaxReqs, 
        MxpsConns, 
        Undefined 
    };

    inline static Values
    stringToEnum(const std::string& name) {
        if (name == MaximumConnections)     return MaxConns;
        if (name == MaximumRequests)        return MaxReqs;
        if (name == MultiplexesConnections) return MxpsConns;
        return Undefined;
    }
    
    inline static std::string
    enumToString(const Values& v) {
        switch (v) {
            case MaxConns:
                return std::string(MaximumConnections);

            case MaxReqs:
                return std::string(MaximumRequests);

            case MxpsConns:
                return std::string(MultiplexesConnections);
            
            default:
                return std::string("undefined");
        }
    }
};

/*
struct UnknownType {
    std::uint8_t    type;
    std::uint8_t    reserved[7];
};
*/


} // fcgi
#endif //__FCGI_HPP
