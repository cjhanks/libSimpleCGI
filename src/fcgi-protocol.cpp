
#include "fcgi-protocol.hpp"
#include <cassert>
#include <iostream>
#include "logging.hpp"


using std::ostream;
using std::string;


namespace fcgi {
namespace {
template <typename _Tp>
static constexpr typename std::enable_if<sizeof(_Tp) == 8, _Tp>::type
endianSwitch(_Tp data) {
        return __builtin_bswap64(data);
}

template <typename _Tp>
static constexpr typename std::enable_if<sizeof(_Tp) == 4, _Tp>::type
endianSwitch(_Tp data) {
    return __builtin_bswap32(data);
}

template <typename _Tp>
static constexpr typename std::enable_if<sizeof(_Tp) == 2, _Tp>::type
endianSwitch(_Tp data) {
#if __GNUC_PREREQ(4, 8)
        return __builtin_bswap16(data);
#else
        return (data << 8) | (data >> 8);
#endif
}

string
headerTypeToString(HeaderType t)
{
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

    return "UNDEFINED";
}
} // ns

/**
 * This function assumes input data is sane according to the FCGI protocol, it
 * can segfault onn bad input
 *
 * @return true
 *      If data input perfectly aligned with the key-value pairs.
 */
bool
readIntoKeyValueMap(const uint8_t* data, size_t len,
                    KeyValueMap& keyValueMap)
{
    assert(data != nullptr);

    const uint8_t* begin = data;
    const uint8_t* end   = data + len;

    auto parseLength = [&]() -> uint32_t {
        if (1 != (*begin) >> 7) {
            assert(begin + 1 <= end);
            return *(begin++);
        } else {
            assert(begin + 4 <= end);
            uint32_t elem(0);
            elem += (*(begin++) & 0x7f) << 24;
            elem += (*(begin++)       ) << 16;
            elem += (*(begin++)       ) << 8;
            elem += (*(begin++)       ) << 0;
            return elem;
        }
    };

    while (begin < end) {
        // parse name length
        size_t keyLength = parseLength();
        size_t valLength = parseLength();

        assert(begin + keyLength + valLength <= end);
        const string key(begin,
                         begin + keyLength);
        const string val(begin + keyLength,
                         begin + keyLength + valLength);

        keyValueMap[key] = val;
        begin += (keyLength + valLength);
    }

    return begin == end;
}

////////////////////////////////////////////////////////////////////////////////
Header::Header()
{
  memset(this, 0, sizeof(*this));
}

bool
Header::isManagementRecord() const
{
    return 0 == requestId;
}

void
Header::switchEndian()
{
    requestId     = endianSwitch(requestId);
    contentLength = endianSwitch(contentLength);
}

ostream&
operator<<(ostream& strm, const Header& h)
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

////////////////////////////////////////////////////////////////////////////////
MessageEndRequest::MessageEndRequest()
{
  memset(this, 0, sizeof(*this));
}

void
MessageBeginRequest::switchEndian()
{
    uint16_t r = static_cast<uint16_t>(role);
    r = endianSwitch(r);
    role = static_cast<RequestRole>(r);
}

bool
MessageBeginRequest::shouldKeepConnection() const
{
    static constexpr size_t KeepConn = 1;
    return flags & KeepConn;
}

ostream&
operator<<(ostream& strm, const MessageBeginRequest& msg)
{
    strm << "Begin:"
         << (size_t) msg.role
         << ":Keep?"
         << msg.shouldKeepConnection();
    return strm;
}

////////////////////////////////////////////////////////////////////////////////

void
MessageEndRequest::switchEndian()
{
    appStatus = endianSwitch(appStatus);
}
} // ns fcgi
