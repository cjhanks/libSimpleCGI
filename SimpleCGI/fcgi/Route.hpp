#ifndef __FCGI_ROUTE_HPP
#define __FCGI_ROUTE_HPP

#include <functional>
#include <map>
#include <string>
#include <set>
#include <vector>

#include "FcgiHttp.hpp"


namespace fcgi {
// {
class HttpRequest;
class HttpResponse;
// }

using Route = std::function<int(HttpRequest&, HttpResponse&)>;
using RouteVector = std::vector<Route>;
using MatchingArgs = std::map<std::string, std::string>;
using VerbSet = std::set<HttpVerb>;


class InstalledRoute {
public:
  InstalledRoute() = default;
  InstalledRoute(const RouteVector& routes,
                 const VerbSet& verbs);

  operator bool() const
  { return routes.size() > 0; }

  bool
  Matches(HttpVerb verb) const;

  void
  Call(HttpRequest& request, HttpResponse& response);

private:
  RouteVector routes;
  VerbSet verbs;

  void
  LogRouteErrorCode(int rc);
};
} // ns fcgi
#endif // __FCGI_ROUTE_HPP
