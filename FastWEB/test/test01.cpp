
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <thread>

#include "fcgi-server.hpp"
#include "fcgi-socket.hpp"
#include "fcgi-io.hpp"
#include "logging.hpp"

using namespace fcgi;

#if __GNUC_PREREQ(4, 8)
std::thread_local pid_t Pid = -1;
#else
__thread pid_t Pid = -1;
#endif


bool
testJSON(HttpRequest& req, HttpResponse& res) {
    if (req.getHeader("content-type") != "application/json") {
        res.setResponse(HttpHeader(300, MimeType::APPLICATION_JSON));
    } else {
        res.setResponse(HttpHeader(200, MimeType::APPLICATION_JSON));
    }
    
    res.write("{\"key\": \"");
    res.write(std::to_string(Pid));
    res.write("\"}");
    return true;
}

void
childInitialized()
{
    std::cerr << "child Initialized" << std::endl;
    Pid = getpid();
}

int
main()
{
    LOG::SetLogLevel(WARNING); // Default value

    ServerConfig config;
    config.concurrencyModel = ServerConfig::ConcurrencyModel::PREFORKED;
    config.childCount = 8;
    config.callBack = childInitialized;

    MasterServer server(config, domainSocket("/tmp/test.sock"));
    server.assets().addSearchPath("/var/www/static", CacheMode::EAGER);
    server.routes().installRoute("/", testJSON);    
    server.dumpTo(std::cerr);

    server.serveForever();
    return 0;
}
