
#include <iostream>
#include "fcgi-server.hpp"
#include "fcgi-socket.hpp"
#include "fcgi-io.hpp"
#include "logging.hpp"
#include "base64.h"

using namespace fcgi;

bool
testIndex(HttpRequest& req, HttpResponse& res) {
    (void) req;
    res.setResponse(HttpHeader(200, MimeType::TEXT_HTML));
    res.write("<p>Hello World</p>");
    return true;
}

bool
testJSON(HttpRequest& req, HttpResponse& res) {
    (void) req;
    res.setResponse(HttpHeader(200, MimeType::APPLICATION_JSON));
    res.write("{\"key\": \"value\"}");
    res.logError("Whats up doc?");
    return true;
}


int
main()
{
    LOG::SetLogLevel(WARNING);

    ServerConfig config;
    config.concurrencyModel = ServerConfig::ConcurrencyModel::PREFORKED;
    config.childCount = 8;
    MasterServer server(config, domainSocket("/tmp/test.sock"));
    server.assets().addSearchPath("/tmp/", CacheMode::LAZY);
    server.assets().addSearchPath("/var/tmp/", CacheMode::EAGER);
    server.routes().installRoute("/", testIndex);
    server.routes().installRoute("/index", testIndex, {HttpVerb::GET});
    server.routes().installRoute("/index/<key>/dog", testIndex, 
                                {HttpVerb::GET, HttpVerb::POST});
    server.routes().installRoute("/json", testJSON, {HttpVerb::GET});
    
    server.dumpTo(std::cerr);

    server.serveForever();
}
