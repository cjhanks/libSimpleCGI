
#include "network-parsing.hpp"
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include "logging.hpp"

using std::begin;
using std::end;
using std::copy;
using std::map;
using std::string;

namespace fcgi {

UniqueSocket::UniqueSocket(int sock)
: socketHandle(sock)
{}

UniqueSocket::~UniqueSocket()
{
    if (socketHandle < 0) {
        close(socketHandle);
    }
}

UniqueSocket::UniqueSocket(UniqueSocket&& rhs)
{
    socketHandle = rhs.socketHandle;
    rhs.socketHandle = -1;
}

Header
UniqueSocket::readHeader()
{
    Header header;
    if (sizeof(header) != ::recv(socketHandle, &header, sizeof(header), 0)) {
        // TODO: throw
    }
    
    header.requestId     = __builtin_bswap16(header.requestId);
    header.contentLength = __builtin_bswap16(header.contentLength);

    return header;
}
    
size_t
UniqueSocket::readIntoBuffer(const Header& header, VectorBuffer& buffer)
{
    struct iovec ioVec[2] = { 
        { 
            .iov_base   = buffer.contentBuffer.data(),
            .iov_len    = header.contentLength
        },
        {
            .iov_base   = buffer.paddingBuffer.data(),
            .iov_len    = header.paddingLength
        }
    };

    ssize_t readLen = readv(socketHandle, ioVec, 2);
    if (readLen != (header.contentLength + header.paddingLength)) {
        // TODO: Throw exception
        buffer.size = 0;
    } else {
        buffer.size = header.contentLength;
        return readLen;
    }

}

void
readBufferIntoKeyValuePair(const VectorBuffer& buffer, KeyValueMap& keyValuePair)
{
    auto begin = buffer.contentBuffer.begin();
    auto end   = begin + buffer.size;

    static auto is4ByteLength = [](const std::uint8_t& character) {
        return 1 == (character >> 7);
    };

    static auto as4ByteLength = [](decltype(begin)& reference) 
                                -> std::uint32_t {
        return ((*(reference++) & 0x7f) << 24) 
             + ((*(reference++)       ) << 16)
             + ((*(reference++)       ) << 8 )
             + ((*(reference++)       ));
    };

    while (begin != end) {
        // parse name length
        size_t nameLength; 
        auto head = begin;
        if (is4ByteLength(*head)) {
            nameLength = as4ByteLength(head);
        } else {
            nameLength = *(head++);
        }
        
        // parse value length
        size_t valueLength;
        if (is4ByteLength(*head)) {
            valueLength = as4ByteLength(head);
        } else {
            valueLength = *(head++);
        }

        // put them into map
        const string key(head , head + nameLength);
        const string val(head + nameLength, 
                         head + nameLength + valueLength);
        LOG(DEBUG) << key << " = " << val;
        keyValuePair[key] = val;
        begin = head + nameLength + valueLength;
    }
}

size_t
saveKeyValuePairIntoBuffer(const KeyValueMap& keyValPair, VectorBuffer& buffer) 
{
    using Iter = decltype(buffer.contentBuffer.begin());
    Iter head = buffer.contentBuffer.data();

    auto fitsIn1Byte = [](const size_t size) {
        return size <= (1 << 7) - 1;
    };

    auto copy1ByteInto = [&](const size_t size) -> Iter {
        std::uint8_t correctSize = static_cast<std::uint8_t>(size);
        return static_cast<Iter>(std::memcpy(head, 
                                             &correctSize, 
                                             sizeof(correctSize)))
             + static_cast<ptrdiff_t>(sizeof(correctSize));
    };
    
    auto copy4ByteInto = [&](const size_t size) -> Iter {
        std::uint32_t correctSize = static_cast<std::uint32_t>(size);
        correctSize = __bswap_32(correctSize);
        return static_cast<Iter>(std::memcpy(head, 
                                             &correctSize, 
                                             sizeof(correctSize)))
             + static_cast<ptrdiff_t>(sizeof(correctSize));
    };

    for (auto& keyValue: keyValPair) {
        // copy key section
        if (fitsIn1Byte(keyValue.first.size())) {
            head = copy1ByteInto(keyValue.first.size());
        } else {
            head = copy4ByteInto(keyValue.first.size());
        }
        
        // copy value section
        if (fitsIn1Byte(keyValue.second.size())) {
            head = copy1ByteInto(keyValue.second.size());
        } else {
            head = copy4ByteInto(keyValue.second.size());
        }

        // copy data
        head = copy(keyValue.first.begin(), 
                    keyValue.first.end(), 
                    head);
        head = copy(keyValue.second.begin(), 
                    keyValue.second.end(), 
                    head);
    }

    return (buffer.size = head - buffer.contentBuffer.begin());
}
} // ns fcgi
