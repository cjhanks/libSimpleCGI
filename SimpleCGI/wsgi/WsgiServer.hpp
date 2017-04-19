#ifndef WSGI_SERVER_HPP_
#define WSGI_SERVER_HPP_

#include "PythonHelper.hpp"
#include "SimpleCGI/fcgi/FcgiServer.hpp"

////////////////////////////////////////////////////////////////////////////////

class WsgiHandler {
public:
  static PyObject* WsgiVersionTuple;

  WsgiHandler(fcgi::HttpRequest& req, fcgi::HttpResponse& res);
  ~WsgiHandler();

  bool
  operator()(PyObject* wsgiFunctor);

private:
  fcgi::HttpRequest& req;
  fcgi::HttpResponse& res;
  PyObject* environment;
  PyObject* callback;

  void
  PopulateEnvironmentCGI();

  void
  PopulateEnvironmentWSGI();

  void
  CreateCallback();
};

////////////////////////////////////////////////////////////////////////////////

class WsgiApplication {
public:
  struct Config {
    std::string module;
    std::string app;
  };

  WsgiApplication(Config config);
  ~WsgiApplication();

  void
  Initialize();

  bool
  Serve(fcgi::HttpRequest& req, fcgi::HttpResponse& res);

private:
  Config config;

  PyObject* module;
  PyObject* application;

  void
  InitializeWSGI_Application();
};

#endif // WSGI_SERVER_HPP_
