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

using Route = std::function<bool(HttpRequest&, HttpResponse&)>;

enum class HttpVerb {
    UNDEFINED = 0,
    ANY,
    GET,
    POST,
    PUT,
    PATCH,
    DELETE
};

std::string 
verbToVerbString(const HttpVerb& verb);

HttpVerb
verbStringToVerb(const std::string& verbStr);

////////////////////////////////////////////////////////////////////////////////

class QueryArgument {
public:
    static QueryArgument
    fromRawString(const std::string& rawString);
   
    QueryArgument() = default;

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
using VerbSet = std::set<HttpVerb>;

class Maybe {
public:
    Maybe();
    Maybe(const Route& matchLink, const VerbSet& verbSet);
    operator bool() const;
    
    template <typename... _Args>
    auto
    call(_Args&&... args) -> decltype(((Route*)(0))->operator()(args...)) {
        return matchLink(args...);
    }

    bool
    matchesVerb(const HttpVerb& verb);
    
    void
    dumpTo(const std::string& prefix, std::ostream&) const;
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
    installRoute(IterType head, IterType last, const Route& route,
                 const VerbSet& verbSet);
    
    Maybe 
    getRoute();
    
    Maybe 
    getRoute(IterType head, IterType last, MatchingArgs* args);

    void
    dumpTo(const std::string& prefix, std::ostream& strm) const;

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
    
    void
    installRoute(const std::string& routeStr, const Route& route,
                 const VerbSet& verbSet);

    Maybe 
    getRoute(const std::string& routeStr, MatchingArgs& args, 
             const HttpVerb& verb);

    friend std::ostream& operator<<(std::ostream&, const MatchingRoot&);

private:
    MatchingLink root;
};
} // ns fcgi

#endif //__FCGI_HTTP_HPP
