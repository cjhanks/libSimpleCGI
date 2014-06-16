#include "fcgi-http.hpp"

#include <cassert>
#include <locale>
#include <sstream>

namespace fcgi {
using std::getline;
using std::locale;
using std::string;
using std::stringstream;
using std::vector;

namespace {
string
urlDecode(const string& input)
{
    static locale loc;

    auto toChar = [](const char c) -> char {
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

Maybe::Maybe(const Route& matchLink)
    : isMatchLink(true), matchLink(matchLink)
{}

Maybe::operator bool()
{ return isMatchLink; }

Maybe::operator Route()
{ return matchLink; }


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


MatchingLink::MatchingLink(IterType head, IterType last, const Route& route)
    : MatchingLink(*head)
{
    installRoute(++head, last, route);
}

void
MatchingLink::installRoute(IterType head, IterType last, const Route& route) 
{
    if (head == last) {
        currentRoute = Maybe(route);
    } else {
        for (auto& ref: matchLinkVector) {
            if (ref.matches(head)) {
                ref.installRoute(++head, last, route);
                return;
            }
        }
        
        matchLinkVector.emplace_back(MatchingLink(head, last, route));
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
    vector<string> routeVec(routeToVector(routeStr));
    root.installRoute(routeVec.cbegin(), routeVec.cend(), route);
}


Route
MatchingRoot::getRoute(const string& routeStr, MatchingArgs& args) 
{
    vector<string> routeVec(routeToVector(routeStr));
    if (0 == routeVec.size()) {
        return root.getRoute();
        } else {
            return root.getRoute(routeVec.cbegin(), routeVec.cend(), &args);
        }
}


} // ns fcgi
