#include "HeaderRouter.hpp"

namespace fcgi {
HeaderRouter::HeaderRouter(const std::string& headerKey)
  : headerKey(headerKey)
{}

HeaderRouter&
HeaderRouter::Add(const std::string& value, const Route& route)
{
  routes[value] = route;
  return *this;
}

HeaderRouter&
HeaderRouter::Fallback(const std::string& value)
{
  fallback = value;
  return *this;
}

int
HeaderRouter::operator()(HttpRequest& request, HttpResponse& response)
{
  // 1.  Retrieve the header
  std::string value = request.GetHeader(headerKey);

  // 2.  Clean the value to be case-insensitive. ?

  // 3.  Retrieve the route.
  auto route = routes.find(value);
  if (route != routes.end())
    return route->second(request, response);

  // 4.  Attempt to fallback.
  route = routes.find(fallback);
  if (route != routes.end())
    return route->second(request, response);

  return -1;
}
} // ns fcgi
