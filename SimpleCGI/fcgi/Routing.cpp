#include "Routing.hpp"

#include <cassert>
#include <iostream>


using std::endl;
using std::ostream;
using std::string;
using std::vector;



namespace fcgi {
Maybe::Maybe()
  : isMatchLink(false)
{}

Maybe::Maybe(const Route& matchLink, const VerbSet& verbSet)
  : isMatchLink(true)
  , matchLink(matchLink)
  , matchVerbs(verbSet)
{}

Maybe::operator bool() const
{
  return isMatchLink;
}

bool
Maybe::MatchesVerb(const HttpVerb& Verb)
{
  return matchVerbs.count(Verb)
      || matchVerbs.count(HttpVerb::ANY);
}

void
Maybe::DumpTo(const string& prefix, ostream& strm) const
{
  for (auto& ref: matchVerbs) {
    strm << "  "
       << VerbToVerbString(ref)
       << endl;
  }

  strm << "  " << prefix << endl;

}

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

  if (head == string::npos && tail == string::npos ) {
    matchType = MatchingType::FIXED;
    matchLink = elem;
  } else {
    matchType = MatchingType::ANY;
    matchLink = elem.substr(head + 1, tail - head - 1);
  }
}


MatchingLink::MatchingLink(IterType head, IterType last, const Route& Route,
               const VerbSet& VerbSet)
  : MatchingLink(*head)
{
  InstallRoute(++head, last, Route, VerbSet);
}

void
MatchingLink::InstallRoute(IterType head, IterType last, const Route& route,
                           const VerbSet& verbSet)
{
  if (head == last) {
    currentRoute = Maybe(route, verbSet);
  } else {
    for (auto& ref: matchLinkVector) {
      if (ref.matches(head)) {
        ref.InstallRoute(++head, last, route, verbSet);
        return;
      }
    }

    matchLinkVector.emplace_back(MatchingLink(head, last, route, verbSet));
  }
}

Maybe
MatchingLink::GetRoute()
{
  return currentRoute;
}

Maybe
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

    return Maybe();
  }
}

void
MatchingLink::DumpTo(const string& prefix, ostream& strm) const
{
  string correctedPrefix = prefix;

  switch (matchType) {
    case MatchingType::ANY:
      correctedPrefix += "/<" + matchLink + ">";
      break;

    default:
      correctedPrefix += matchLink;
      break;
  }

  if (currentRoute) {
    strm << string(80, '-') << endl;
    currentRoute.DumpTo(correctedPrefix, strm);
  }

  for (auto& ref: matchLinkVector) {
    ref.DumpTo(correctedPrefix, strm);
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
MatchingRoot::InstallRoute(const string& RouteStr, const Route& Route)
{
  return InstallRoute(RouteStr, Route, {HttpVerb::ANY});
}

void
MatchingRoot::InstallRoute(const string& RouteStr, const Route& Route,
               const VerbSet& VerbSet)
{
  vector<string> RouteVec(RouteToVector(RouteStr));
  root.InstallRoute(RouteVec.cbegin(), RouteVec.cend(), Route, VerbSet);
}

Maybe
MatchingRoot::GetRoute(const string& RouteStr, MatchingArgs& args,
             const HttpVerb& Verb)
{
  vector<string> RouteVec(RouteToVector(RouteStr));
  Maybe maybeRoute;
  if (0 == RouteVec.size()) {
    maybeRoute = root.GetRoute();
  } else {
    maybeRoute = root.GetRoute(RouteVec.cbegin(), RouteVec.cend(), &args);
  }

  if (maybeRoute && maybeRoute.MatchesVerb(Verb)) {
    return maybeRoute;
  } else {
    return Maybe();
  }
}

ostream&
operator<<(ostream& strm, const MatchingRoot& root)
{
  root.root.DumpTo("", strm);
  return strm;
}
} // ns fcgi
