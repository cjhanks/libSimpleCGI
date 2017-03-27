<!-- vim: ts=2 sw=2 ai tw=72 et
  -->

# Brief

  libSimpleCGI is a C++ library targeting Linux/Unix systems
implementing most of the FastCGI protocol.  It can optionally include a
WSGI server for hosting Python3 applications (such as Flask/Bottle).


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

You might also find it useful that your WSGI Python3 applications can
run inside the same executable as your C++ code.


# Quickstart

## Build

Dependencies:

  - C++11
  - Python3-dev (if you want WSGI support)


```bash
  $> scons -j4
  # OR
  $> scons -j4 --wsgi
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
