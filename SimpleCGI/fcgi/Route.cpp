#include "Route.hpp"

#include "SimpleCGI/common/Logging.hpp"


namespace fcgi {
InstalledRoute::InstalledRoute(
        const RouteVector& routes, const VerbSet& verbs)
  : routes(routes)
  , verbs(verbs)
{
  LOG::CHECK(verbs.size())
      << "At least 1 verb must be defined.";

  LOG::CHECK(routes.size() > 0)
      << "More than 1 route required";

  for (const auto& route: routes)
    LOG::CHECK(!!route)
      << "Invalid route provided";
}

bool
InstalledRoute::Matches(HttpVerb verb) const
{
  return verbs.count(verb)
      || verbs.count(HttpVerb::ANY);
}

void
InstalledRoute::Call(HttpRequest& request, HttpResponse& response)
{
  int rc = 0;
  for (const auto& route: routes) {
    rc |= route(request, response);
    if (rc)
      break;
  }

  LogRouteErrorCode(rc);
}

void
InstalledRoute::LogRouteErrorCode(int rc)
{
  // TODO
  (void) rc;
}
} // ns fcgi
