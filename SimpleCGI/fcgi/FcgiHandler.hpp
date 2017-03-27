#ifndef __FCGI_HANDLER_HPP
#define __FCGI_HANDLER_HPP

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "FcgiProtocol.hpp"
#include "FcgiHttp.hpp"
#include "FcgiStream.hpp"
#include "FcgiMimetype.hpp"
#include "SimpleCGI/common/Logging.hpp"
#include "bits/FcgiHandler.hpp"


namespace fcgi {
class HttpRequest;
class MasterServer;
class LogicalApplicationSocket;


class HttpRequest {
public:
  HttpRequest(LogicalApplicationSocket* client);

  HttpVerb
  Verb() const { return httpVerb; }

  std::string
  Route() const { return httpRoute; }

  /// {@
  /// All key-value accessors have the behavior of returning an empty string
  /// when the key cannot be found.  No exceptions will be thrown for missing
  /// values.

  // Access an HTTP header sent by the upstream FCGI server
  // the query will be converted to all upper case.
  std::string
  GetHeader(std::string key,
        const std::string& defaultValue = "") const;

  const KeyValueMap&
  headers() const
  { return httpHeaders; }

  // Access a query argument from the url:
  // ?query=value
  const QueryArgument&
  Arguments() const
  { return queryArgs; }

  // When the Route was installed it may have had variable Arguments
  // eg: /path/<Route>/here
  //   /path/example/here
  // Here "Route" is the key, "example" is the value
  std::string
  GetRouteArgument(const std::string& key);
  /// @}

  // NOTE: Currently this is not a "cheap" function, as it performs string parsing on
  // every operation.
  size_t
  ContentLength() const;

  ////////////////////////////////////////////////////////////////////////////
  // POST / PUT / PATCH
  ////////////////////////////////////////////////////////////////////////////
  size_t
  RecvAll(std::vector<std::uint8_t>& data);

  size_t
  RecvAll(std::string& data);

  template <typename _Tp>
  size_t
  recv(_Tp* data, size_t count) {
    return recv(reinterpret_cast<std::uint8_t*>(data),
          count * sizeof(_Tp));
  }

  Istream
  ToStream();

  size_t
  recv(std::uint8_t* data, size_t len);

  ////////////////////////////////////////////////////////////////////////////
  // DELETE
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // DEBUG & INTERNAL
  ////////////////////////////////////////////////////////////////////////////
  Maybe
  GetRoute(MasterServer* master);

  void
  DumpRequestDebugTo(std::ostream& strm);

private:
  std::string httpRoute;
  KeyValueMap httpHeaders;
  MatchingArgs matchingArgs;
  QueryArgument queryArgs;
  HttpVerb httpVerb;
  LogicalApplicationSocket* client;

  //
  size_t bytesRead;
};

////////////////////////////////////////////////////////////////////////////////
class HttpHeader {
public:
  explicit HttpHeader(const size_t& responseCode);
  HttpHeader(const size_t& responseCode, const std::string& responseString);

  void
  addHeader(const std::string& key, const std::string& val);
  operator std::string() const;

private:
  size_t responseCode;
  std::string responseString;
  std::map<std::string, std::string> headers;
};

class HttpResponse {
public:
  HttpResponse(LogicalApplicationSocket* client);
  ~HttpResponse();

  void
  LogError(const std::string& message);

  void
  SetResponse(const HttpHeader& header);

  Ostream
  ToStream();

  size_t
  write(const std::string& data);

  template <typename _Tp>
  size_t
  write(const _Tp* data, size_t count) {
    return write(reinterpret_cast<const std::uint8_t*>(data),
           count * sizeof(_Tp));
  }

  size_t
  write(const std::uint8_t* data, size_t len);

  void
  Close();

private:
  LogicalApplicationSocket* client;
};
} // ns fcgi

#endif //__FCGI_HANDLER_HPP
