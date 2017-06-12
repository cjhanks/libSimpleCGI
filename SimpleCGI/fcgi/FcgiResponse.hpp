#ifndef __FCGI_RESPONSE_HPP_
#define __FCGI_RESPONSE_HPP_

#include <cstdint>
#include <map>
#include <string>


namespace fcgi {
// {
class LogicalApplicationSocket;
class Ostream;
class HttpResponse;

namespace bits {
class HttpResponseStreamBuf;
} // ns bits
// }


class HttpHeader {
public:
  /// Construct the HttpHeader response with an error code using the
  /// builtin ErrorCode->ErrorString logic.
  explicit HttpHeader(const size_t& responseCode);

  /// Construct the HttpHeader response with a content-type
  /// Example:
  ///   HttpHeader(200, "application/json")
  HttpHeader(const size_t& responseCode, const std::string& contentType);

  /// At a header to the HttpHeader, examples:
  /// AddHeader("content-type", "application/json")
  void
  Add(const std::string& key, const std::string& val);

private:
  size_t responseCode;
  std::string responseString;
  std::map<std::string, std::string> headers;

  friend class HttpResponse;
  operator std::string() const;
};

// -------------------------------------------------------------------------- //

class HttpResponse {
public:
  virtual ~HttpResponse();

  /// This method must be called before sending any other data through the
  /// response stream. Failure to do-so is undefined behavior.
  void
  SetResponse(const HttpHeader& header);

  /// Extract the Ostream from this request.  Functionally this is a noop on
  /// the stream.  This method can be called repeatedly and all Ostream
  /// objects will share the underlying stream.  Therefore it is preferred
  /// to have one open at a time.
  Ostream
  ToStream();

  /// Logs an error according to the protocol specification.  In the case of
  /// FastCGI, this logs a message on the `HeaderType::STDERR` channel which
  /// will usually show up in the error logs of your webserver.
  void
  LogError(const std::string& message);

private:
  LogicalApplicationSocket* client;

  // --
  friend class MasterServer;
  friend class bits::HttpResponseStreamBuf;

  explicit HttpResponse(LogicalApplicationSocket* client);

  template <typename _Tp>
  size_t
  write(const _Tp* data, size_t count) {
    return write(reinterpret_cast<const std::uint8_t*>(data),
           count * sizeof(_Tp));
  }

  size_t
  write(const std::string& data);

  size_t
  write(const std::uint8_t* data, size_t len);

  void
  Close();
};
} // ns fcgi

#endif // __FCGI_RESPONSE_HPP_
