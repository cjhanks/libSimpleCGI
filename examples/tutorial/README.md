# Tutorial

The entry point of the tutorial can be found in `Tutorial.cpp`.

## Building

To build this tutorial, from the root of the repository run.

    $> scons --examples

    # For WSGI example
    $> scons --examples --wsgi

You can then find and run the binary in:

    $> ${GIT_ROOT}/build/{release|native|debug}/examples/tutorial/main


## Running

You should follow the NGINX configuration instructions in `examples/README.md`
first.

If you are running without WSGI enabled, you can simply run the command.  **If
you are using WSGI** you need to do a few more things.


### Running with WSGI

This is the easiest way to get started.

    # FROM THE PROJECT ROOT
    $> scons --examples --wsgi
    $> pushd examples/tutorial/
    $> python3 -m virtualenv venv
    $> . venv/bin/activate
    $> pip install bottle
    $> export PYTHONPATH=${PYTHONPATH}:${PWD}
    $> popd
    $> ./build/release/examples/tutorial/main
    Python loaded

    # From `Part1/Routes.cpp::HelloWorld(...)`
    $> curl localhost:8000/hello/world/cpp
    Hello C++ world
    
    # From `simplecgi_example.py::hello(...)`
    $> curl localhost:8000/hello/world/python
    Hello Python world
