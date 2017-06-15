
# Brief

  libSimpleCGI is a statically linkable library which implements the
FastCGI protocol and optionally the WSGI application specification.

  The primary objective is to make development of web-services in the
C++ language straight forward to *write* and *deploy*.  The secondary
objective is to make the integration of C++ and Python web services
possible.

## Status

  Alpha quality and soliciting feedback.

| Branch | Status
|:-------|:---------------------------------------------------------------------
| next   | ![next](https://api.travis-ci.org/cjhanks/libSimpleCGI.svg?branch=next)
| master | ![master](https://api.travis-ci.org/cjhanks/libSimpleCGI.svg?branch=master)


# Quickstart

## Tested Platforms

| Platform    | GCC | CLANG
|:------------|:----|:----------------------------------------------------------
| Ubuntu 14.04| NO  | MAYBE (needs newer version than default libstdc++)
| Ubuntu 16.04| YES | YES
| Ubuntu 17.04| YES | UNKNOWN

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

  # To build the WSGI library and associated examples.
  $> scons --wsgi

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

## Install

```bash

  # FROM THE ROOT OF THE DIRECTORY
  $> scons install \
       --mode={native|release} \
       --install-prefix /path/to/install \
       [--wsgi]

  $> tree /path/to/install
  ├── include
  │   └── SimpleCGI
  │       ├── ...
  │       │   └── ...
  │       └── SimpleCGI.hpp
  └── lib
      └── libSimpleCGI.a

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

# Why libSimpleCGI?

  In my experience - a WSGI Python application (Flask/Bottle/...) fulfills the
majority of my web-server performance needs.

However, there are cases where using C++ for a given route simply makes
life easier.

  - The route will need to parse/interpret a byte stream with non-trivial
    semantics.
    - Ie: Conditional branching on structured types.
    - Ie: Converting marshalled data to a different consumable form.
  - The route will need to execute a significant number of mathematical
    operations
    - Ie: Applying transformations on numerical data.
    - Ie: Statistically summarizing a large stored data.
  - C++ code already exists for a specific operation which would required a
    rewrite/porting to Python.

Historically I solved these problems in the following ways:

  - Create a C++ foreign-function-interface via one of the various wrappers;
    SWIG, Boost.Python, Cython, etc.
  - Create a C library which exposes a Ctypes/CFFI interface to my C++ library.
  - Create a Python C-Extension which directly uses the `PyObject*` type
    system.

Each strategy works, though they carry different baggage.

libSimpleCGI allows you to create a statically linked C++ program capable of
running your C++ routes and your WSGI routes in the same process(1).


(1) It is expected that `libpython3.so` will most likely be a dynamic link.


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
