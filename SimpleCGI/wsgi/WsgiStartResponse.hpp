#ifndef WSGI_START_RESPONSE_HPP_
#define WSGI_START_RESPONSE_HPP_

#include "PythonHelper.hpp"
#include "SimpleCGI/fcgi/FcgiServer.hpp"


PyObject*
New(fcgi::HttpResponse* res);

#endif
