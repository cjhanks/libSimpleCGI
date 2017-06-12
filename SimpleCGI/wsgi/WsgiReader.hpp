#ifndef WSGI_READER_HPP_
#define WSGI_READER_HPP_

#include "PythonHelper.hpp"
#include "SimpleCGI/fcgi/FcgiServer.hpp"

namespace fcgi {

PyObject*
New(fcgi::HttpRequest* req);

} // ns fcgi

#endif
