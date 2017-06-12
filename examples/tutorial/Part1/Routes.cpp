////////////////////////////////////////////////////////////////////////////////
// Part installing basic `GET` routes.
////////////////////////////////////////////////////////////////////////////////

#include "Routes.hpp"


namespace Part1 {
namespace {
using Request = fcgi::HttpRequest;
using Response = fcgi::HttpResponse;


//
// A simple health check GET request intended to simply respond "OK"
// followed by all of the headers sent.
//
int
HealthCheck(Request& request, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));

  fcgi::Ostream ostream = response.ToStream();
  ostream << "OK\n";

  // Dump all of the headers.
  for (const auto& header: request.Headers()) {
    ostream << header.first << " = "
            << header.second << "\n";
  }

  // Grab a specific header.  Header access it not case sensitive,
  // all of the below `GetHeader` calls perform the same function.
  ostream << "----------------------------------\n"
          << request.GetHeader("HTTP_USER_AGENT") << "\n"
          << request.GetHeader("http_user_agent") << "\n"
          << request.GetHeader("http-user-agent") << "\n";

  return 0;
}

//
// Extract basic route attributes and echo them back out to the user.
//
int
BasicRouteRequest(Request& request, Response& response)
{
  std::string resource = request.GetRouteArgument("resource");
  std::string attribute = request.GetRouteArgument("attribute");

  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));

  fcgi::Ostream ostream = response.ToStream();
  ostream << resource << " is " << attribute;

  return 0;
}

int
AmbiguityFish(Request&, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));
  response.ToStream() << "This unambiguously was a fish";
  return 0;
}

int
AmbiguityAny(Request& request, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));
  response.ToStream() << "This was a "
                      << request.GetRouteArgument("resource");
  return 0;
}

//
// Part of how to access query args, this will assume
// that the query arg string is sent:
//
//  ?key=words
//
//  FIXME:
//    Presently the query arg implementation does not support
//    repeated arguments.
//
int
QueryArgs(Request& request, Response& response)
{
  const fcgi::QueryArgument& args = request.QueryArguments();
  std::string key = args.GetArgument("key");
  std::string value = args.GetArgument("value", "default");

  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));

  response.ToStream()
        << "key = " << key << "; "
        << "value = " << value;
  return 0;
}
} // ns

// ========================================================================== //

void
Install(fcgi::MasterServer& server)
{
  server.InstallRoute(
    "/example1/healthCheck",
    HealthCheck,
    {fcgi::HttpVerb::GET}
  );

  server.InstallRoute(
    "/example1/resource/fish",
    AmbiguityFish,
    {fcgi::HttpVerb::GET}
  );

  server.InstallRoute(
    "/example1/resource/<resource>",
    AmbiguityAny,
    {fcgi::HttpVerb::GET}
  );

  server.InstallRoute(
    "/example1/resource/<resource>/attribute/<attribute>",
    BasicRouteRequest,
    {fcgi::HttpVerb::GET}
  );

  server.InstallRoute(
    "/example1/queryArgs",
    QueryArgs,
    {fcgi::HttpVerb::GET}
  );
}
} // ns Part1
