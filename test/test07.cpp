
#include "fcgi-server.hpp"
#include "fcgi-socket.hpp"
#include "fcgi-io.hpp"


namespace {
bool
handle(fcgi::HttpRequest& req, fcgi::HttpResponse& res)
{
    using namespace fcgi;

    fcgi::HttpHeader header(201, "application/text");
    header.addHeader("Transfer-Encoding", "chunked");
    res.setResponse(header);

    std::string body;
    req.recvAll(body);

    LOG(INFO) << body;

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

    server.routes().installRoute(
        "/fcgi",
        handle,
        {fcgi::HttpVerb::POST});
    server.serveForever();

    return 0;
}
