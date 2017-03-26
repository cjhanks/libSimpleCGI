#ifndef __FCGI_PROTOCOL_HPP
#define __FCGI_PROTOCOL_HPP

#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <type_traits>


namespace fcgi {
#if 0
static constexpr size_t MaximumContentDataLen = (1 << 16);
#else
static constexpr size_t MaximumContentDataLen = (1 << 12);
#endif
static constexpr size_t MaximumPaddingDataLen = (1 <<  8);


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

using RequestID = std::uint16_t;

struct Header {
    Header();

    Version         version;
    HeaderType      type;
    RequestID       requestId;
    std::uint16_t   contentLength;
    std::uint8_t    paddingLength;
    std::uint8_t    reserved;

    bool
    isManagementRecord() const;

    void
    switchEndian();

    friend std::ostream& operator<<(std::ostream&, const Header&);
} __attribute__((packed));


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

    void
    switchEndian();

    bool
    shouldKeepConnection() const;

    friend std::ostream& operator<<(std::ostream&, const MessageBeginRequest&);
} __attribute__((packed));

enum class ProtocolStatus : std::uint8_t {
    REQUEST_COMPLETE    = 1,
    CANT_MPX_CONN       = 2,
    OVERLOADED          = 3,
    UNKNOWN_ROLE        = 4
};

struct MessageEndRequest {
    MessageEndRequest();

    std::uint32_t       appStatus;
    ProtocolStatus      protocolStatus;
    std::uint8_t        reserved[3];

    void
    switchEndian();
} __attribute__((packed));

struct MessageUnknown {
} __attribute__((packed));

static_assert(sizeof(MessageEndRequest) == 8
            , "Invalid End request size");

} // ns fcgi

#endif //__FCGI_PROTOCOL_HPP
