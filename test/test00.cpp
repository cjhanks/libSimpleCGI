
#include <iostream>
#include "fcgi-server.hpp"
#include "fcgi-socket.hpp"
#include "base64.h"

using namespace fcgi;

bool
testValue(HttpRequest& req, HttpResponse& res) {
    using namespace std;
    req.dumpRequestDebugTo(std::cerr);
    
    uint8_t* data = new uint8_t[1024];
    cerr << "START" << endl;

    HttpHeader responseHeader(200, "text/html");
    responseHeader.addHeader("key", "value");
    res.setResponse(responseHeader);
    
    size_t tot = 0;
    size_t len;
    while (len = req.recv(+data, 1024)) {
        //res.write(base64_encode(data, len));
        tot += len;
    }

    for (size_t i = 0; i < 1000; ++i) {
        res.write("HEY WORLD\n");
    }

    cerr << "WE DONE: " << tot << endl;
    //string buff(1024 * 68, 'A');
    ////for (size_t i = 0; i < 1024 * 1024; ++i) {
    //for (size_t i = 0; i < 1000; ++i) {
    //    res.write(buff);
    //}
    //if (req.verb() == HttpVerb::GET) {
    //    res.write("WAS A GET!");
    //}

    return true;
}

int
main()
{
    ServerConfig config;
    MasterServer server(config, domainSocket("/tmp/test.sock"));
    server.HttpRoutes.installRoute("/test/path/<name>", testValue);
    server.serveForever();
}
