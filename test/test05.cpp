
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
thread_local pid_t Pid = -1;
#else
__thread pid_t Pid = -1;
#endif

class RouteHandler {
public:
    bool
    operator()(HttpRequest& req, HttpResponse& res)
    {
        switch (req.verb()) {
            case HttpVerb::GET:
                return this->get(req, res);

            case HttpVerb::POST:
            case HttpVerb::PUT:
                return this->put(req, res);

            default:
                return true;
        }
    }

    bool
    get(HttpRequest& req, HttpResponse& res)
    {
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

    bool
    put(HttpRequest& req, HttpResponse& res)
    {
        (void) req;
        res.setResponse(HttpHeader(404, MimeType::APPLICATION_JSON));
        return true;
    }
};

void
childInitialized()
{
    std::cerr << "child Initialized" << std::endl;
    Pid = getpid();
}

int
main()
{
    LOG::SetLogLevel(DEBUG); // Default value

    ServerConfig config;
    config.concurrencyModel = ServerConfig::ConcurrencyModel::THREADED;
    config.childCount = 3;
    config.callBack = childInitialized;

    MasterServer server(config, tcpSocket("127.0.0.1", 8000));
    server.assets().addSearchPath("/var/www/static", CacheMode::EAGER);
    server.routes().installRoute("/fcgi", RouteHandler());
    server.dumpTo(std::cerr);

    server.serveForever();
    return 0;
}
