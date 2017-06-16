#ifndef __FCGI_ROUTING_HPP
#define __FCGI_ROUTING_HPP

#include <functional>
#include <map>
#include <string>
#include <set>

#include "FcgiHttp.hpp"
#include "Route.hpp"


namespace fcgi {
// {
class HttpRequest;
class HttpResponse;
// }


class MatchingLink {
  enum class MatchingType {
    FIXED,
    ANY,
    ROOT,
    UNDEFINED
  };

public:
  using IterType = typename std::vector<std::string>::const_iterator;
  using ElemType = std::string;

  static MatchingLink
  getRoot();

  MatchingLink(ElemType elem);
  MatchingLink(IterType head, IterType last, const InstalledRoute& route);

  void
  InstallRoute(IterType head, IterType last, const InstalledRoute& route);

  InstalledRoute
  GetRoute();

  InstalledRoute
  GetRoute(IterType head, IterType last, MatchingArgs* args);

private:
  InstalledRoute currentRoute;
  ElemType matchLink;
  MatchingType matchType;

  std::vector<MatchingLink> matchLinkVector;

  bool
  matches(IterType elem, MatchingArgs* args = nullptr);
};


class MatchingRoot {
public:
  MatchingRoot();

  void
  InstallRoute(const std::string& routeStr, const InstalledRoute& route);

private:
  MatchingLink root;

  friend class HttpRequest;
  InstalledRoute
  GetRoute(const std::string& RouteStr, MatchingArgs& args,
           const HttpVerb& Verb);

};
} // ns fcgi

#endif // __FCGI_ROUTING_HPP
