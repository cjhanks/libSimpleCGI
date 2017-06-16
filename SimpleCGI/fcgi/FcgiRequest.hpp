#ifndef __FCGI_REQUEST_HPP_
#define __FCGI_REQUEST_HPP_

#include <cstdint>
#include <string>

#include "FcgiHttp.hpp"
#include "FcgiProtocol.hpp"
#include "Routing.hpp"


namespace fcgi {
// {
class LogicalApplicationSocket;
class MasterServer;
class Istream;

namespace bits {
class HttpRequestStreamBuf;
} // ns bits
// }

/// Note:
///   All key-value accessors have the behavior of returning an empty string
///   when the key cannot be found.  No exceptions will be thrown for missing
///   values.
class HttpRequest {
public:
  /// Returns the HTTP Verb associated with this request.
  HttpVerb
  Verb() const { return httpVerb; }

  /// Returns a normalized route path string for this request.
  std::string
  Route() const { return httpRoute; }

  /// Access an HTTP header sent by the upstream FCGI server
  /// the query will be converted to all upper case.
  std::string
  GetHeader(std::string key,
            const std::string& defaultValue = "") const;

  /// Return the key-value map of all headers.  This type can always
  /// be treated like a std::map.
  const KeyValueMap&
  Headers() const
  { return httpHeaders; }

  // Returns the QueryArgument object associated with this request.
  //
  // For something like:
  //  ?key=1&that=words
  //
  // const auto& args = request.QueryArguments();
  // std::string key = args.GetArgument("key");
  //
  // if (key.size() == 0)
  //  // NO QUERY ARGUMENT FOUND
  const QueryArgument&
  QueryArguments() const
  { return queryArgs; }

  // When the Route was installed it may have had variable Arguments
  // Route: /path/<route>/here
  //   GET: /path/example/here
  //
  // assert(request.GetRouteArgument("route") == "example");
  std::string
  GetRouteArgument(const std::string& key, std::string defaultValue = "");

  /// @}

  // NOTE: Currently this is not a "cheap" function, as it performs string
  // parsing on every operation.
  size_t
  ContentLength() const;

  /// Extract the Istream from this request.  Functionally this is a noop on
  /// the stream.  This method can be called repeatedly and all Istream
  /// objects will share the underlying stream.  Therefore it is preferred
  /// to have one open at a time.
  Istream
  ToStream();

  void
  DumpRequestDebugTo(std::ostream& strm);

private:
  std::string httpRoute;
  KeyValueMap httpHeaders;
  MatchingArgs matchingArgs;
  QueryArgument queryArgs;
  HttpVerb httpVerb;
  LogicalApplicationSocket* client;

  size_t bytesRead;

  // ------------------------------------------------------------------------ //
  friend class MasterServer;
  friend class bits::HttpRequestStreamBuf;

  explicit HttpRequest(LogicalApplicationSocket* client);

  InstalledRoute
  GetRoute(MasterServer* master);

  /// {
  /// Receive functions
  template <typename _Tp>
  size_t
  recv(_Tp* data, size_t count) {
    return recv(reinterpret_cast<std::uint8_t*>(data),
                count * sizeof(_Tp));
  }

  size_t
  recv(std::uint8_t* data, size_t len);
  /// }
};
} // ns fcgi

#endif // __FCGI_REQUEST_HPP_
