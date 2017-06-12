#ifndef WSGI_START_RESPONSE_HPP_
#define WSGI_START_RESPONSE_HPP_

#include "PythonHelper.hpp"
#include "SimpleCGI/SimpleCGI.hpp"

namespace fcgi {

PyObject*
New(fcgi::HttpResponse* res);

} // ns fcgi

#endif
