#ifndef __SYNCHRONOUS_REQUEST_HPP
#define __SYNCHRONOUS_REQUEST_HPP 

#include "fcgi.hpp"
#include "network-parsing.hpp"


namespace fcgi {

class SynchronousRequest {
public:
    SynchronousRequest(Version version, RequestID requestId, Begin beginMessage,
                       VectorBuffer buffer);

    std::string
    getHeader(const std::string& key) const;

    std::size_t
    receive(std::uint8_t* data, size_t len);

private:
    const Version fcgiVersion;
    const RequestID requestID;
    UniqueSocket requestSocket;
    VectorBuffer buffer;
};

class SynchronousResponderRequest : public SynchronousRequest {
public:
    
};

} // fcgi

#endif //__SYNCHRONOUS_REQUEST_HPP
