#ifndef BITS_FCGI_HANDLER_HPP_
#define BITS_FCGI_HANDLER_HPP_

#include <array>
#include <iostream>


namespace fcgi {
class HttpRequest;
class HttpResponse;


namespace bits {
class HttpRequestStreamBuf : public std::streambuf {
public:
  HttpRequestStreamBuf(HttpRequest* request);
  virtual std::streambuf::int_type underflow();

private:
  std::array<char, 4096> buffer;
  HttpRequest* request;
};


class HttpResponseStreamBuf : public std::streambuf {
public:
  HttpResponseStreamBuf(HttpResponse* request);
  virtual ~HttpResponseStreamBuf();

  virtual std::streambuf::int_type overflow(std::streambuf::int_type value);
  virtual int sync();

private:
  std::array<char, 4096> buffer;
  HttpResponse* response;
};
} // ns bits
} // ns fcgi

#endif // BITS_FCGI_HANDLER_HPP_
