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

    operator bool() { return socketHandle >= 0; }

    Header 
    readHeader();

    size_t
    readIntoBuffer(const Header& header, VectorBuffer& buffer);
    
private:
    int socketHandle;
};

void
readBufferIntoKeyValuePair(const VectorBuffer& buffer, KeyValueMap& keyValuePair);

size_t
saveKeyValuePairIntoBuffer(const KeyValueMap& keyValPair, VectorBuffer& buffer);


} // ns fcgi

#endif // __NETWORKING_PARSING_HPP
