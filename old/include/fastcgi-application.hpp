#ifndef __FASTCGI_APPLICATION_HPP
#define __FASTCGI_APPLICATION_HPP

#include <string>
#include <set>
#include "fcgi.hpp"
#include "network-parsing.hpp"

namespace fcgi {
////////////////////////////////////////////////////////////////////////////////
struct DomainSocket {
public:
    DomainSocket(const std::string& hostPath);
    ~DomainSocket();
    operator int();

private:
    const std::string hostPath;
    int sockFD;
};

////////////////////////////////////////////////////////////////////////////////
class FastCGI {
public:
    FastCGI(int serverSocket);
    ~FastCGI();

    void 
    enterRequestLoop();
    
private:
    static KeyValueMap defaultGetValueMap;
    int serverSocket;
    std::set<std::string> serverAddresses;

    void
    handleManagementRecord(const Header& header, UniqueSocket& sock);

    void
    handleApplicationRequest(const Header& head, UniqueSocket& sock);
};

} // fcgi

#endif //__FASTCGI_APPLICATION_HPP
