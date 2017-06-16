#ifndef MIDDLEWARE_HEADER_ROUTER_HPP_
#define MIDDLEWARE_HEADER_ROUTER_HPP_

#include <string>

#include "SimpleCGI/fcgi/Fcgi.hpp"


namespace fcgi {
class HeaderRouter {
public:
  explicit HeaderRouter(const std::string& headerKey);

  HeaderRouter&
  Add(const std::string& value, const Route& route);

  HeaderRouter&
  Fallback(const std::string& value);

  int
  operator()(HttpRequest& request, HttpResponse& response);

private:
  std::string headerKey;
  std::string fallback;
  std::map<std::string, Route> routes;
};
} // ns fcgi


#endif // MIDDLEWARE_HEADER_ROUTER_HPP_
