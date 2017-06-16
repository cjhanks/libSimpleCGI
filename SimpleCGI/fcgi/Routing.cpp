#include "Routing.hpp"

#include <cassert>
#include <iostream>

#include "SimpleCGI/common/Logging.hpp"


using std::endl;
using std::ostream;
using std::string;
using std::vector;



namespace fcgi {
MatchingLink
MatchingLink::getRoot()
{
  MatchingLink root("/");
  root.matchType = MatchingType::ROOT;
  return root;
}


MatchingLink::MatchingLink(ElemType elem)
  : matchLink(elem), matchType(MatchingType::UNDEFINED)
{
  size_t head = elem.find_first_of("<");
  size_t tail = elem.find_last_of(">");

  if (head == string::npos
   || tail == string::npos ) {
    matchType = MatchingType::FIXED;
    matchLink = elem;
  } else {
    matchType = MatchingType::ANY;
    matchLink = elem.substr(head + 1, tail - head - 1);
  }
}


MatchingLink::MatchingLink(
    IterType head, IterType last, const InstalledRoute& route)
  : MatchingLink(*head)
{
  InstallRoute(++head, last, route);
}

void
MatchingLink::InstallRoute(IterType head, IterType last, const InstalledRoute& route)
{
  if (head == last) {
    currentRoute = route;
  } else {
    for (auto& ref: matchLinkVector) {
      if (ref.matches(head)) {
        ref.InstallRoute(++head, last, route);
        return;
      }
    }

    matchLinkVector.emplace_back(MatchingLink(head, last, route));
  }
}

InstalledRoute
MatchingLink::GetRoute()
{
  return currentRoute;
}

InstalledRoute
MatchingLink::GetRoute(IterType head, IterType last, MatchingArgs* args)
{
  if (head == last) {
    return currentRoute;
  } else {
    for (auto& ref: matchLinkVector) {
      if (ref.matches(head, args)) {
        return ref.GetRoute(++head, last, args);
      }
    }

    return InstalledRoute();
  }
}

bool
MatchingLink::matches(IterType elem, MatchingArgs* args)
{
  switch (matchType) {
    case MatchingType::ROOT:
      return true;

    case MatchingType::FIXED:
      return *elem == matchLink;

    case MatchingType::ANY:
      if (args) {
        // at this point there is a '/' in the first character, however
        // it's not useful for the matching args.
        assert(elem->front() == '/');
        args->insert(make_pair(matchLink, elem->substr(1)));
      }
      return true;

    case MatchingType::UNDEFINED:
      return false;
  }

  return false;
}

namespace {
vector<string>
RouteToVector(const string& Route) {
  vector<string> vec;
  size_t head = Route.find_first_of("/");
  while (head != string::npos) {
    size_t tail = Route.find_first_of("/", head + 1);
    string data(Route.substr(head, tail - head));
    if (data.size() > 1) {
      vec.emplace_back(data);
    }
    head = tail;
  }

  return vec;
}
} // ns

MatchingRoot::MatchingRoot()
  : root(MatchingLink::getRoot())
{}

void
MatchingRoot::InstallRoute(const string& routeStr, const InstalledRoute& route)
{
  vector<string> RouteVec = RouteToVector(routeStr);
  root.InstallRoute(RouteVec.cbegin(), RouteVec.cend(), route);
}

InstalledRoute
MatchingRoot::GetRoute(const string& routeStr, MatchingArgs& args,
                       const HttpVerb& verb)
{
  vector<string> routeVec(RouteToVector(routeStr));
  InstalledRoute maybeRoute;
  if (0 == routeVec.size()) {
    maybeRoute = root.GetRoute();
  } else {
    maybeRoute = root.GetRoute(routeVec.cbegin(), routeVec.cend(), &args);
  }

  if (maybeRoute && maybeRoute.Matches(verb)) {
    return maybeRoute;
  } else {
    return InstalledRoute();
  }
}
} // ns fcgi
