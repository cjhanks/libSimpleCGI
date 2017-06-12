////////////////////////////////////////////////////////////////////////////////
// Part installing a basic `POST`, `PUT`, `PATCH` route.
//------------------------------------------------------------------------------
//
// Virtual all serialization/deserialization C++ libraries will in some way
// support an interface for `std::istream/std::ostream`.  Those libraries can
// act on the request/response
////////////////////////////////////////////////////////////////////////////////

#include "Routes.hpp"


namespace Part2 {
namespace {
using Request = fcgi::HttpRequest;
using Response = fcgi::HttpResponse;


//
// Simply echoes whatever was POST/PUT-ed from the user.
//
// XXX
// Keep in mind that most FCGI servers will fully buffer the user input before
// sending the first byte of output.  As a consequence, this convention of
// sending output before the last byte of output has arrived is ill-advised and
// may have negative consequences.
//
// The example is simply trying to show the behavior of the Istream/Ostream
// classes can be used anywhere that a `std::istream&/std::ostream&` can.
// XXX
//
int
Echo(Request& request, Response& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));

  fcgi::Istream istream = request.ToStream();
  fcgi::Ostream ostream = response.ToStream();

  if (ostream << istream.rdbuf())
    return 0;
  else
    return 1;
}
} // ns

// ========================================================================== //

void
Install(fcgi::MasterServer& server)
{
  server.InstallRoute(
    "/example2/echo",
    Echo,
    {fcgi::HttpVerb::PATCH,
     fcgi::HttpVerb::POST,
     fcgi::HttpVerb::PUT}
  );
}
} // ns Part2
