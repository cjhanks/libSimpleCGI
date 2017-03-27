#ifndef FCGI_HANDLER_HPP_
#define FCGI_HANDLER_HPP_

#include <iostream>
#include <memory>

#include "bits/FcgiHandler.hpp"


namespace fcgi {
class HttpRequest;
class HttpResponse;

class Istream : public std::istream {
public:
  explicit Istream(HttpRequest* req)
    : std::istream(&buffer)
    , buffer(req)
  {}

  Istream(Istream&& rhs)
    : std::istream(std::move(rhs))
    , buffer(std::move(rhs.buffer))
  {}

private:
  bits::HttpRequestStreamBuf buffer;
};


class Ostream : public std::ostream {
public:
  explicit Ostream(HttpResponse* req)
    : std::ostream(&buffer)
    , buffer(req)
  {}

  Ostream(Ostream&& rhs)
    : std::ostream(std::move(rhs))
    , buffer(std::move(rhs.buffer))
  {}

private:
  bits::HttpResponseStreamBuf buffer;
};
} // ns fcgi

#endif // FCGI_HANDLER_HPP_
