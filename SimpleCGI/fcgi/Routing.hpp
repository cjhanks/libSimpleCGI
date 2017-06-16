#ifndef __FCGI_ROUTE_HPP
#define __FCGI_ROUTE_HPP

#include <functional>
#include <map>
#include <string>
#include <set>

#include "FcgiHttp.hpp"



namespace fcgi {
// {
class HttpRequest;
class HttpResponse;
// }

using Route = std::function<int(HttpRequest&, HttpResponse&)>;
using MatchingArgs = std::map<std::string, std::string>;
using VerbSet = std::set<HttpVerb>;

class Maybe {
public:
  Maybe();
  Maybe(const Route& matchLink, const VerbSet& verbSet);
  operator bool() const;

  template <typename... _Args>
  auto
  call(_Args&&... args) -> decltype(((Route*)(0))->operator()(args...))
  { return matchLink(args...); }

  bool
  MatchesVerb(const HttpVerb& Verb);

  void
  DumpTo(const std::string& prefix, std::ostream&) const;

private:
  bool isMatchLink;
  Route matchLink;
  VerbSet matchVerbs;
};

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
  MatchingLink(IterType head, IterType last, const Route& route,
               const VerbSet& verbSet);

  void
  InstallRoute(IterType head, IterType last, const Route& route,
               const VerbSet& verbSet);

  Maybe
  GetRoute();

  Maybe
  GetRoute(IterType head, IterType last, MatchingArgs* args);

  void
  DumpTo(const std::string& prefix, std::ostream& strm) const;

private:
  Maybe currentRoute;
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
  InstallRoute(const std::string& routeStr, const Route& route);

  void
  InstallRoute(const std::string& routeStr, const Route& route,
               const VerbSet& verbSet);


  friend std::ostream& operator<<(std::ostream&, const MatchingRoot&);

private:
  MatchingLink root;

  friend class HttpRequest;
  Maybe
  GetRoute(const std::string& RouteStr, MatchingArgs& args,
           const HttpVerb& Verb);

};
} // ns fcgi

#endif // __FCGI_ROUTE_HPP
