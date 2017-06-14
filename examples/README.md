# examples

## Quickstart

Read the `NGINX` section and configure your system this way.  All demos use the
same configuration and can therefore not be ran concurrently.

## NGINX

Example FCGI application's are listening on `tcp:127.0.0.1:9000`.

The following NGINX configuration will create a server listening on `*:8000`
using `127.0.0.1:9000` as an upstream using the FastCGI protocol.

    # /etc/nginx/sites-enabled/simplecgi_example
    server {
      listen 8000;
      listen [::]:8000;

      location / {
        # or whatever your distro has named this file.
        include /etc/nginx/fastcgi_params;

        fastcgi_pass  127.0.0.1:9000;
        # For domain sockets
        #fastcgi_pass  unix:/tmp/test.sock;
      }
    }


## Examples

### hello_world

A nearly minimum possible functioning application.


### tutorial

A walk through which exercises every function call which might be relevant to
building an application using SimpleCGI.
