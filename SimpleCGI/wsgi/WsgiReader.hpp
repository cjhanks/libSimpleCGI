#ifndef WSGI_READER_HPP_
#define WSGI_READER_HPP_

#include "PythonHelper.hpp"
#include "SimpleCGI/fcgi/FcgiServer.hpp"


PyObject*
New(fcgi::HttpRequest* req);

#endif
