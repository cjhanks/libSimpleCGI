<!-- vim: ts=2 sw=2 ai tw=72 et
  -->

# Brief

  libSimpleCGI is a C++ library targeting Linux/Unix systems
implementing most of the FastCGI protocol.  It will soon optionally
include a WSGI server for hosting Python3 applications
(such as Flask/Bottle).


## Why libSimpleCGI?

  If you have a bunch of code already written in C++ which would be
useful for a web server, there are only a few practical integration
strategies.

  1.  Use a full-fledged webserver in your codebase.  There are a few
      relatively good ones... unfortunately all of the good ones are
      fairly heavy in dependencies.
  2.  Embed your C++ into a higher level language (such as Python) by
      making calls through an FFI.

libSimpleCGI does not have all of great features of a fully featured
webserver such as HTTP/2 and it is not an event-based framework.
Rather, it relies on NGINX to provide a lot of the functionality built
into webservers.  If all you need is reasonably performant server
without the dependency headache's, it might work for you.

Presently support of integrating Python3 WSGI applications is under development.


# Quickstart

## Build

Dependencies:

  - C++ compiler with C+11 or later.
  - Linux/Unix system (it's unclear if MacOSX will work)
  - Python3-dev (if you want WSGI support)


```bash
  # Library only
  $> scons

  # To build the examples
  $> scons --examples

  # Modes:
  #   release: Generic binary which is portable independent of
  #            CPU architecture [DEFAULT]
  #   native:  Targeted against this native architecture.
  #   debug:   Enables address sanitizers and other debug info.
  $> scons --examples --mode=release
  $> scons --examples --mode=debug
  $> scons --examples --mode=native

  # To compile using Clang
  export CXX=clang++
  $> scons ...
```

## Hello 'World'

Using the NGINX config found in `examples/tutorial/README.md` sets up a minimal
SimpleCGI application which can be queried:

    $> curl localhost:8000/hello/world
    Hello world

The code...

```cpp
#include <SimpleCGI/SimpleCGI.hpp>


namespace {
int
Hello(fcgi::HttpRequest& request, fcgi::HttpResponse& response)
{
  response.SetResponse(fcgi::HttpHeader(200, "text/plain"));
  fcgi::Ostream ostream = response.ToStream();
  ostream << "Hello " << request.GetRouteArgument("name");
  return 0;
}
} // ns

int
main(void)
{
  fcgi::ServerConfig config;
  fcgi::MasterServer server(config, fcgi::TcpSocket("127.0.0.1", 9000));

  server.InstallRoute(
    "/hello/<name>",
    Hello,
    {fcgi::HttpVerb::GET}
  );

  return server.ServeForever();
}
```

## Tutorial

Please see `examples/tutorial/README.md` for example usage.

Additionally, you may run the examples in the tutorial if you build them.  They
will be placed into `./build/{release|debug|native}/examples/{project}/`
directory.

```bash
  $> scons --examples
```

# Compliance

## FastCGI

  The library presently only implements the `RESPONDER` role for
`Application` records.  Theoretically everything but section 4
`Management` records is presently possible.

  In any case, that's enough to be useful in conjunction with NGINX.

  [Specification](https://htmlpreview.github.io/?https://github.com/FastCGI-Archives/fcgi2/blob/master/doc/fcgi-spec.html#S4)

## WSGI

  PEP-0333 is implemented caveat a few things:

  1.  "The server or gateway must transmit the yielded strings to the
      client in an unbuffered fashion".

      That is not the case in this implementation for a few reasons;
      NGINX buffers AND the libSimpleCGI implementation also buffers.

  [Specification](https://www.python.org/dev/peps/pep-0333/)
