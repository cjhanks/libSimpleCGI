#ifndef __NETWORKING_PARSING_HPP
#define __NETWORKING_PARSING_HPP 

#include <cstdint>
#include <array>
#include <map>
#include <string>

#include "fcgi.hpp"

namespace fcgi {

class UniqueSocket {
public:
    UniqueSocket(int sock);
    ~UniqueSocket();

    UniqueSocket(const UniqueSocket&) = delete;
    UniqueSocket&
    operator=(const UniqueSocket&) = delete;

    UniqueSocket(UniqueSocket&& rhs);
    
    operator int() { return this->socketHandle; }
    
private:
    int socketHandle;
};

struct VectorBuffer {
    ssize_t readInto(const Header& header, UniqueSocket& socket);
    std::array<std::uint8_t, MaximumContentDataLen> contentBuffer;
    std::array<std::uint8_t, MaximumPaddingDataLen> paddingBuffer;
    size_t size;
};

using KeyValueMap = std::map<std::string, std::string>;

KeyValueMap
convertBufferToKeyValuePair(const VectorBuffer& buffer);

size_t
saveKeyValuePairIntoBuffer(const KeyValueMap& keyValPair, VectorBuffer& buffer);


} // ns fcgi

#endif // __NETWORKING_PARSING_HPP
