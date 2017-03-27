#ifndef WSGI_START_RESPONSE_HPP_
#define WSGI_START_RESPONSE_HPP_

#include <Python.h>
#include "SimpleCGI/fcgi/FcgiServer.hpp"


PyObject*
New(fcgi::HttpResponse* res);

#endif
