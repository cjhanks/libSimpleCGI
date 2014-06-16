FastWEB
=======

FastWEB was written for back-end web application developers with low-latency
requirements &&|| developers with computationally expensive operations which are
simply unsuitable for more traditional FCGI languages (Python, Perl, Ruby).

In brief: 
    FastWEB is an attempt to bring the simplicity of web development from
    Pythonic frameworks such-as Bottle/Flask to C++.

The most basic useful C++ FCGI application might looke like this:

```C++
#include <string>
#include <vector>
#include <fcgi/fcgi.hpp>

using std::string;
using std::vector;

bool route_0(HttpRequest& req, HttpResponse& res);
bool route_1(HttpRequest& req, HttpResponse& res);
bool route_2(HttpRequest& req, HttpResponse& res);

int
main(void)
{
    fcgi::ServerConfig config;
    MasterServer serve(config, domainSocket("/tmp/domain.sock"));
    serve.installRoute("/", route_0);
    serve.installRoute("/path/absolute/truth", route_1);
    serve.installRoute("path/<routeName>/truth", route_2);
    serve.serveForever();

    return 0; // never reached
}

bool 
route_0(HttpRequest& req, HttpResponse& res)
{   
    vector<uint_t> allData;
    req.readAll(allData);
    if (allData.size() < 32) {
        req.setResponse(HttpResponse(400, "text/html"));
    } else {
        req.setResponse(HttpResponse(400, "text/html"));
    }

    res.write("<html>");
    res.write("<body>");
    res.write("Never tell me: ")
    res.write(allData);
    res.write(" again!");
    res.write("</body>");
    res.write("</html>");

    return true;
}
```
