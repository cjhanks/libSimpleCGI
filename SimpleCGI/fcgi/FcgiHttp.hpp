#ifndef __FCGI_HTTP_HPP
#define __FCGI_HTTP_HPP

////////////////////////////////////////////////////////////////////////////////
// This file contains an assortment of smaller standalone classes useful for
// interpreting and representing HTTP data.

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fcgi {
class HttpRequest;
class HttpResponse;

using Route = std::function<int(HttpRequest&, HttpResponse&)>;

enum class HttpVerb {
  UNDEFINED = 0,
  ANY,
  GET,
  POST,
  PUT,
  PATCH,
  DELETE
};

/// DEPRECATED
/// XXX: This should be removed and replaced by the below `ToString(...)`.
std::string
VerbToVerbString(const HttpVerb& Verb);

inline std::string
ToString(const HttpVerb& verb)
{ return VerbToVerbString(verb); }

HttpVerb
VerbStringToVerb(const std::string& VerbStr);

////////////////////////////////////////////////////////////////////////////////

class QueryArgument {
  using QueryArgMap = std::map<std::string, std::string>;

public:
  using const_iterator = typename QueryArgMap::const_iterator;

  QueryArgument() = default;

  std::string
  GetArgument(const std::string& key,
              const std::string& defaultValue = "") const;

  /// {
  /// Allows the structure to be treated like a std::map.
  const_iterator
  find(const std::string& key) const
  { return queryArgs.find(key); }

  const_iterator
  begin() const
  { return queryArgs.begin(); }

  const_iterator
  end() const
  { return queryArgs.end(); }
  /// }

private:
  friend class HttpRequest;
  QueryArgMap queryArgs;
  QueryArgument(const QueryArgMap& queryArgMap);

  static QueryArgument
  fromRawString(const std::string& rawString);
};

////////////////////////////////////////////////////////////////////////////////

using MatchingArgs = std::map<std::string,std::string>;
using VerbSet = std::set<HttpVerb>;

class Maybe {
public:
  Maybe();
  Maybe(const Route& matchLink, const VerbSet& VerbSet);
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

#endif //__FCGI_HTTP_HPP
