////////////////////////////////////////////////////////////////////////////////
// Installation of middleware
////////////////////////////////////////////////////////////////////////////////

#include "Routes.hpp"


namespace Part5 {
namespace {
using Request = fcgi::HttpRequest;
using Response = fcgi::HttpResponse;

// {
int
HelloWorld_0(Request&, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));
  return 0;
}

int
HelloWorld_1(Request&, Response& response)
{
  response.ToStream() << "Hello Pipelined ";
  return 0;
}

int
HelloWorld_2(Request& request, Response& response)
{
  std::string name = request.GetRouteArgument("name");
  response.ToStream() << name;
  return 0;
}
// }

// {
int
ContentType_JSON(Request&, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "application/json"));
  response.ToStream()
      << "{\"message\": \"Content type was JSON\"}";
  return 0;
}

int
ContentType_TEXT(Request&, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));
  response.ToStream()
      << "Content Type was TEXT";
  return 0;
}
// }
} // ns

// ========================================================================== //

void
Install(fcgi::MasterServer& server)
{
  server.InstallRoute(
    "/example5/hello/<name>",
    {
      HelloWorld_0,
      HelloWorld_1,
      HelloWorld_2
    },
    {fcgi::HttpVerb::GET}
  );

  server.InstallRoute(
    "/example5/content-type",
    fcgi::HeaderRouter("content-type")
         .Add("application/json",
              ContentType_JSON)
         .Add("text/plain",
              ContentType_TEXT)
         .Fallback("text/plain"),
    {fcgi::HttpVerb::GET}
  );

}
} // ns Part5
