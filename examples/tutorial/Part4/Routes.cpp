////////////////////////////////////////////////////////////////////////////////
// Demonstrates how to install the experimental WSGI server.
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
#if WITH_WSGI == 1

#include "Routes.hpp"

#include <functional>
#include <memory>


namespace Part4 {
namespace {
std::unique_ptr<fcgi::WsgiApplication> wsgi;
} // ns

// ========================================================================== //



void
Install(fcgi::ServerConfig& config)
{
  // {
  // Configure the Python information necessary for importing the WSGI
  // application.
  fcgi::WsgiApplication::Config wsgiConfig;
  wsgiConfig.module = "simplecgi_example";
  wsgiConfig.app = "app";
  // }

  // {
  // The fcgi::WsgiApplication needs to have a life time which is longer
  // than the fcgi::MasterServer.  This idiom of saving it in a static
  // local may not be ideal, but it is sufficient for this tutorial.
  wsgi.reset(new fcgi::WsgiApplication(wsgiConfig));;
  // }

  // {
  // The callbacks described in tutorial Part3 are here used to install
  // the WSGI application.
  //
  // The WsgiApplication::Initialize() function is necessary for
  // initializing the Python3 runtime.
  //
  // The WsgiApplication::Server(...) function is responsible for routing
  // traffic from C++-land into Python and performing the necessary WSGI
  // specification.  This implies that routes will first attempt to be
  // served by the C++ routes and will only fall back to WSGI if it cannot
  // be fulfilled.  If the route is not valid in Python land, it is the
  // responsibility of the WSGI application to implement the catch-all
  // error handler.
  config.callBack = std::bind(&fcgi::WsgiApplication::Initialize, wsgi.get());
  config.catchAll = std::bind(&fcgi::WsgiApplication::Serve,
                              wsgi.get(),
                              std::placeholders::_1,
                              std::placeholders::_2);
  // }
}
} // ns Part4

#endif // WITH_WSGI == 1
