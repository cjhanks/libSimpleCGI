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
} // ns fcgi

#endif //__FCGI_HTTP_HPP
