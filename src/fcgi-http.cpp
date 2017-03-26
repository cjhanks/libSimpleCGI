#include "fcgi-http.hpp"

#include <cassert>
#include <locale>
#include <sstream>
#include "logging.hpp"

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
verbStringToVerb(const string& verbStr) 
{
    if (verbStr == "GET")    return HttpVerb::GET;
    if (verbStr == "POST")   return HttpVerb::POST;
    if (verbStr == "PUT")    return HttpVerb::PUT;
    if (verbStr == "PATCH")  return HttpVerb::PATCH;
    if (verbStr == "DELETE") return HttpVerb::DELETE;

    return HttpVerb::UNDEFINED;
}

string 
verbToVerbString(const HttpVerb& verb)
{
    switch (verb) {
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
        size_t mids = element.find_first_of("=", head);

        // there is no '=' sign
        if (mids == string::npos) {
            queryArgs[urlDecode(element)] = "";
        } else {
            queryArgs[urlDecode(element.substr(0, mids))] = 
                      urlDecode(element.substr(mids + 1));
        }

        head = tail + 1;
    }

    return QueryArgument(queryArgs);
}

QueryArgument::QueryArgument(const QueryArgMap& queryArgMap)
    : queryArgs(queryArgMap)
{}

string
QueryArgument::getArgument(const string& key, const string& defaultValue) const
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

Maybe::Maybe(const Route& matchLink, const VerbSet& verbSet)
    : isMatchLink(true), matchLink(matchLink), matchVerbs(verbSet)
{}

Maybe::operator bool() const
{ 
    return isMatchLink; 
}
    
bool
Maybe::matchesVerb(const HttpVerb& verb)
{
    return matchVerbs.count(verb)
        || matchVerbs.count(HttpVerb::ANY);
}

void
Maybe::dumpTo(const string& prefix, ostream& strm) const
{
    for (auto& ref: matchVerbs) {
        strm << "    "
             << verbToVerbString(ref)
             << endl;
    }

    strm << "    " << prefix << endl;
    
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


MatchingLink::MatchingLink(IterType head, IterType last, const Route& route,
                           const VerbSet& verbSet)
    : MatchingLink(*head)
{
    installRoute(++head, last, route, verbSet);
}

void
MatchingLink::installRoute(IterType head, IterType last, const Route& route,
                           const VerbSet& verbSet)
{
    if (head == last) {
        currentRoute = Maybe(route, verbSet);
    } else {
        for (auto& ref: matchLinkVector) {
            if (ref.matches(head)) {
                ref.installRoute(++head, last, route, verbSet);
                return;
            }
        }
        
        matchLinkVector.emplace_back(MatchingLink(head, last, route, verbSet));
    }
}

Maybe 
MatchingLink::getRoute()
{
    return currentRoute; 
}
    
Maybe 
MatchingLink::getRoute(IterType head, IterType last, MatchingArgs* args) 
{
    if (head == last) {
        return currentRoute; 
    } else {
        for (auto& ref: matchLinkVector) {
            if (ref.matches(head, args)) {
                return ref.getRoute(++head, last, args);
            }
        }
        
        return Maybe();
    }
}
    
void
MatchingLink::dumpTo(const string& prefix, ostream& strm) const
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
        currentRoute.dumpTo(correctedPrefix, strm);
    }

    for (auto& ref: matchLinkVector) {
        ref.dumpTo(correctedPrefix, strm);
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
routeToVector(const string& route) {
    vector<string> vec;
    size_t head = route.find_first_of("/");
    while (head != string::npos) {
        size_t tail = route.find_first_of("/", head + 1);
        string data(route.substr(head, tail - head));
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
MatchingRoot::installRoute(const string& routeStr, const Route& route)
{
    return installRoute(routeStr, route, {HttpVerb::ANY});
}

void
MatchingRoot::installRoute(const string& routeStr, const Route& route,
                           const VerbSet& verbSet)
{
    vector<string> routeVec(routeToVector(routeStr));
    root.installRoute(routeVec.cbegin(), routeVec.cend(), route, verbSet);
}

Maybe
MatchingRoot::getRoute(const string& routeStr, MatchingArgs& args,
                       const HttpVerb& verb)
{
    vector<string> routeVec(routeToVector(routeStr));
    Maybe maybeRoute;
    if (0 == routeVec.size()) {
        maybeRoute = root.getRoute();
    } else {
        maybeRoute = root.getRoute(routeVec.cbegin(), routeVec.cend(), &args);
    }

    if (maybeRoute && maybeRoute.matchesVerb(verb)) {
        return maybeRoute;
    } else {
        return Maybe();
    }
}
    
ostream& 
operator<<(ostream& strm, const MatchingRoot& root)
{
    root.root.dumpTo("", strm);
    return strm;
}
} // ns fcgi
