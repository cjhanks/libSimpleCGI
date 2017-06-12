////////////////////////////////////////////////////////////////////////////////
// Demonstrates using a few SimpleCGI features which are not FastCGI or HTTP
// routing related.
//------------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

#include "Routes.hpp"


namespace Part3 {
namespace {
using Request = fcgi::HttpRequest;
using Response = fcgi::HttpResponse;

void
OnInitialized()
{
  // In `THREADED` mode, this code can now safely modify thread_local data.
  //
  // In `PREFORKED` mode, this code can now safely modify global data.
  fcgi::LOG(fcgi::INFO) << "Initialized";
}


int
OnNoRouteMatch(Request& request, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(404, "text/plain"));
  response.ToStream() << "No such resource known '"
                      << fcgi::ToString(request.Verb()) << ": "
                      << request.Route() << "'";
  return 0;
}
} // ns

// ========================================================================== //

void
Install(fcgi::ServerConfig& config)
{
  // {
  // The registered callback is called by every child before accepting
  // any sockets to do work on.  It is called in all ConcurrencyModels.
  //
  // In THREADED code, this callback is executed in each child thread.
  // In PREFORKED code, this callback is executed in each child process.
  // In SYNCHRONOUS code, this callback is executed by the main thread
  // before performing any `accept` calls on the socket.
  //
  config.callBack = OnInitialized;
  // }

  // {
  // This registers a callback to executed when no roue is matched by
  // the HTTP request.
  //
  // It may be implemented as a generic 404 handler.  If no call back
  // is installed it defaults to sending a 404 request with no message
  // body.
  config.catchAll = OnNoRouteMatch;
  // }
}
} // ns Part3
