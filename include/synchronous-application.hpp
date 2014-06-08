#ifndef __SYNCHRONOUS_APPLICATION_HPP
#define __SYNCHRONOUS_APPLICATION_HPP

#include <set>
#include "synchronous-request.hpp"


namespace fcgi {

struct DomainSocket {
public:
    DomainSocket(const std::string& hostPath);
    ~DomainSocket();
    operator int();

private:
    const std::string hostPath;
    int sockFD;
};

class SynchronousApplication {
public:
    SynchronousApplication(int serverSocket);
    
    std::string
    name() const;

    SynchronousRequest*
    acceptRequest();
    
private:
    static KeyValueMap defaultGetValueMap;
    int serverSocket;
    const std::string applicationName;
    std::set<std::string> serverAddresses;

    void
    handleManagementRecord(const Header& header, UniqueSocket& sock);

    SynchronousRequest*
    establishedApplicationRequest(const Header& head, UniqueSocket& sock);
};

} // fcgi

#endif //__SYNCHRONOUS_APPLICATION_HPP
