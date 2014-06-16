#ifndef __FCGI_HTTP_HPP
#define __FCGI_HTTP_HPP

////////////////////////////////////////////////////////////////////////////////
// This file contains an assortment of smaller standalone classes useful for
// interpreting and representing HTTP data.

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace fcgi {
class HttpRequest;
class HttpResponse;

using Route = std::function<bool(HttpRequest&, HttpResponse&)>;

enum class HttpVerb {
    UNDEFINED,
    GET,
    POST,
    PUT,
    PATCH,
    DELETE
};

inline HttpVerb
verbStringToVerb(const std::string& verbStr) 
{
    if (verbStr == "GET")    return HttpVerb::GET;
    if (verbStr == "POST")   return HttpVerb::POST;
    if (verbStr == "PUT")    return HttpVerb::PUT;
    if (verbStr == "PATCH")  return HttpVerb::PATCH;
    if (verbStr == "DELETE") return HttpVerb::DELETE;

    return HttpVerb::UNDEFINED;
}

inline std::string 
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
    
        default:
            return "UNDEFINED";
    }
}


////////////////////////////////////////////////////////////////////////////////
class QueryArgument {
public:
    QueryArgument() {}
    static QueryArgument
    fromRawString(const std::string& rawString);

    std::string
    getArgument(const std::string& key,
                const std::string& defaultValue = "") const;
private:
    using QueryArgMap = std::map<std::string, std::string>;
    friend class HttpRequest;
    QueryArgMap queryArgs;
    QueryArgument(const QueryArgMap& queryArgMap);
};
    
////////////////////////////////////////////////////////////////////////////////

using MatchingArgs = std::map<std::string,std::string>;

class Maybe {
public:
    Maybe();
    Maybe(const Route& matchLink);
    operator bool();
    operator Route();
    
    Route&
    route() { return matchLink; }
private:
    bool isMatchLink;
    Route matchLink;
};

class MatchingLink {
    enum class MatchingType {
        FIXED,
        ANY, 
        ROOT,
        UNDEFINED
    };

public:
    using SelfType = MatchingLink;
    using IterType = typename std::vector<std::string>::const_iterator;
    using ElemType = std::string;

    static MatchingLink
    getRoot();
    
    MatchingLink(ElemType elem);
    MatchingLink(IterType head, IterType last, const Route& route);

    void
    installRoute(IterType head, IterType last, const Route& route);
    
    Maybe 
    getRoute();
    
    Maybe 
    getRoute(IterType head, IterType last, MatchingArgs* args);

    void
    dump(int indent = 0) {
        for (auto& link: matchLinkVector) {
            link.dump(indent + 1);
        }
    }

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
    installRoute(const std::string& routeStr, const Route& route);

    Route
    getRoute(const std::string& routeStr, MatchingArgs& args);

    void
    dump() {
        root.dump();
    }
private:
    MatchingLink root;
};
} // ns fcgi

#endif //__FCGI_HTTP_HPP
