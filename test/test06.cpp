
#include "fcgi-server.hpp"
#include "fcgi-socket.hpp"
#include "fcgi-io.hpp"


namespace {
bool
handle(fcgi::HttpRequest& req, fcgi::HttpResponse& res)
{
    fcgi::HttpHeader header(201, "application/text");
    header.addHeader("Transfer-Encoding", "chunked");
    res.setResponse(header);

    (void) req;
    char buffer[1024] = {0};
    for (size_t i = 0; i < 1024 * 1024; ++i)
    {
      res.write(buffer, 1024);
    }

    return true;
}
} // ns

int
main()
{
    fcgi::LOG::SetLogLevel(fcgi::DEBUG); // Default value

    fcgi::ServerConfig config;
    config.concurrencyModel =
        fcgi::ServerConfig::ConcurrencyModel::THREADED;
    config.childCount = 4;

    fcgi::MasterServer server(
        config,
        fcgi::tcpSocket("127.0.0.1", 8000));

    server.routes().installRoute("/fcgi", handle);
    server.serveForever();

    return 0;
}
