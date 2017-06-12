
#include <SimpleCGI/SimpleCGI.hpp>


namespace {
int
Hello(fcgi::HttpRequest& request, fcgi::HttpResponse& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));
  fcgi::Ostream ostream = response.ToStream();
  ostream << "Hello " << request.GetRouteArgument("name");
  return 0;
}
}

int
main(void)
{
  fcgi::ServerConfig config;
  fcgi::MasterServer server(config, fcgi::TcpSocket("127.0.0.1", 9000));

  server.InstallRoute(
    "/hello/<name>",
    Hello,
    {fcgi::HttpVerb::GET}
  );

  return server.ServeForever();
}
