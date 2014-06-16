#ifndef __FCGI_RESPONDER_HPP
#define __FCGI_RESPONDER_HPP

#include "fcgi.hpp"
#include "network-parsing.hpp"

namespace fcgi {
class FCGI_Responder {
public:
    FCGI_Responder(const Header& firstHeader, const Begin beginMessage);
                   
    void
    operator()(UniqueSocket& socket, VectorBuffer& buffer);

    std::string&
    requestURI() const;

    std::string&
    queryString() const;

private:
    const Version version;
    const RequestID requestId;
    const Begin beginMessage;
    KeyValueMap httpParams;

    void 
    assertIsSecureHeader(const Header& header);
};
} // ns fcgi

#endif //__FCGI_RESPONDER_HPP
