#include "FcgiHttp.hpp"

#include <cassert>
#include <locale>
#include <sstream>
#include "SimpleCGI/common/Logging.hpp"

namespace fcgi {
using std::endl;
using std::getline;
using std::locale;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

namespace {
string
urlDecode(const string& input)
{
  static locale loc;

  static auto toChar = [](const char c) -> char {
    if (c >= '0' && c <= '9') {
      return c - '0';
    } else if (c >= 'A' && c <= 'F') {
      return c - 'A';
    } else {
      // THROW
      return 0;
    }
  };

  stringstream ostrm;
  for (size_t i = 0; i < input.size(); ++i) {
    switch (input[i]) {
      case '%':
        if (i + 2 < input.size()) {
          ostrm << static_cast<char>(toChar(input[i + 1]) * 16
                       + toChar(input[i + 2]));
          i += 2;
        } else {
          // THROW
        }
        break;

      case '+':
        break;

      default:
        if (isalnum(input[i], loc)) {
          ostrm << input[i];
        } else {
          // THROW
        }
        break;
    }
  }

  return ostrm.str();
}
} // ns

HttpVerb
VerbStringToVerb(const string& VerbStr)
{
  if (VerbStr == "GET")  return HttpVerb::GET;
  if (VerbStr == "POST")   return HttpVerb::POST;
  if (VerbStr == "PUT")  return HttpVerb::PUT;
  if (VerbStr == "PATCH")  return HttpVerb::PATCH;
  if (VerbStr == "DELETE") return HttpVerb::DELETE;

  return HttpVerb::UNDEFINED;
}

string
VerbToVerbString(const HttpVerb& Verb)
{
  switch (Verb) {
    case HttpVerb::UNDEFINED:
      return "UNDEFINED";

    case HttpVerb::GET:
      return "GET";

    case HttpVerb::POST:
      return "POST";

    case HttpVerb::PUT:
      return "PUT";

    case HttpVerb::PATCH:
      return "PATCH";

    case HttpVerb::DELETE:
      return "DELETE";

    case HttpVerb::ANY:
      return "ANY";

    default:
      return "UNDEFINED";
  }
}

////////////////////////////////////////////////////////////////////////////////

QueryArgument
QueryArgument::fromRawString(const string& rawString)
{
  static string Separators("&;");
  QueryArgMap queryArgs;

  size_t head = 0;
  size_t tail = 0;
  while (tail != string::npos) {
    tail = rawString.find_first_of(Separators, head);
    string element(rawString.substr(head, tail - head));
    size_t mids = element.find_first_of("=");

    std::string key;
    std::string value;

    if (mids == string::npos) {
      // there is no '=' sign
      key = urlDecode(element);
    } else {
      key = urlDecode(element.substr(0, mids));
      value = urlDecode(element.substr(mids + 1));
    }

    queryArgs[key] = value;
    head = tail + 1;
  }

  return QueryArgument(queryArgs);
}

QueryArgument::QueryArgument(const QueryArgMap& queryArgMap)
  : queryArgs(queryArgMap)
{}

string
QueryArgument::GetArgument(const string& key, const string& defaultValue) const
{
  auto it = queryArgs.find(key);
  if (it == queryArgs.end()) {
    return defaultValue;
  } else {
    return it->second;
  }
}

////////////////////////////////////////////////////////////////////////////////

Maybe::Maybe()
  : isMatchLink(false)
{}

Maybe::Maybe(const Route& matchLink, const VerbSet& VerbSet)
  : isMatchLink(true), matchLink(matchLink), matchVerbs(VerbSet)
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
