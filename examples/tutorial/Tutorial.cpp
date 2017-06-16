
#include <SimpleCGI/SimpleCGI.hpp>

#include "Part1/Routes.hpp"
#include "Part2/Routes.hpp"
#include "Part3/Routes.hpp"

#if WITH_WSGI == 1
#include "Part4/Routes.hpp"
#endif

#include "Part5/Routes.hpp"



int
main(void)
{
  // Changes the amount of stdout logging, all loglevels are
  // inclusive of the items listed below it.
  //  DEBUG
  //  INFO
  //  WARNING
  //  ERROR
  //  FATAL
  //  NONE
  fcgi::LOG::SetLogLevel(fcgi::WARNING);
  //fcgi::LOG::SetLogLevel(fcgi::DEBUG);

  // {
  // Create the server configuration
  fcgi::ServerConfig config;

  // There are 3 concurrency models:
  //  SYNCHRONOUS:
  //    Only a single request is handled at a time, and it is
  //    handled in the same thread that
  //    fcgi::MasterServer::ServeForever()` is called in.
  //
  //    When set:
  //      `config.childCount` is meaningless.
  //
  //  THREADED:
  //    Creates a pool of reusable `config.childCount` threads. Note that
  //    there is always `config.childCount + 1` threads.  This is because
  //    one thread is exclusively responsible for `accept` calls on the
  //    socket and may be used in various background tasks.
  //
  //    Threads are *not* detached from the main process.  Abort calls in
  //    any thread will terminate all threads.
  //
  //    Important:
  //      Child sockets are passed through a thread-safe blocking queue.
  //      This queue can theoretically overflow, it is expected that
  //      queuing is performed in the downstream FastCGI provider.
  //
  //  PREFORKED:
  //    There are `config.childCount + 1` processes.  The main process
  //    is responsible for accepting sockets and sending them to children
  //    in different processes.
  //
  //    Crashes in the main thread will cause an abort of all children,
  //    connections will not be drained.
  //
  //    Crashes in child threads will be restarted by main thread.
  //
  config.concurrencyModel =
      fcgi::ServerConfig::ConcurrencyModel::SYNCHRONOUS;
  config.childCount = 1;
  // }

  // {
  // At this point the tutorial diverges and has a few different
  // configuration settings which might be useful.
  //
  // It is advised you first continue to the end of the tutorial
  // and then return to this point.  No features in this block
  // are required knowledge.

#if WITH_WSGI != 1
  // Demonstrates how to implement the basic callbacks which
  // can be registered in the lifecycle of the server.
  Part3::Install(config);
#else
  // Tutorial on using `catchAll` callback to route requests to a
  // Python WSGI server.
  //
  // To use this tutorial you will need to modify the code to find
  // and import your WSGI application.
  //
  // XXX: Presently the WSGI path is not supported, it has a few
  //      small issues which need to be debugged.
  Part4::Install(config);
#endif
  // }

  // {
  // Create the webserver socket.  The user is *not* responsible for closing
  // this socket.  Theoretically any socket can be passed into the server,
  // however there are two helpers; Tcp and Domain.
  //
  int socket = fcgi::TcpSocket("127.0.0.1", 9000);

  // alternatively create a domain socket.
  //int server = fcgi::DomainSocket("/var/tmp/fcgi.sock");
  // }

  // {
  // Create the server.
  fcgi::MasterServer server(config, socket);
  // }

  // {
  // At this point the tutorial separates different logical components into
  // different `{ns}::Install` functions.

  // Demonstrates how routes are resolved using HTTP:GET operations.
  Part1::Install(server);

  // Demonstrates how to perform POST/PUT body requests.
  Part2::Install(server);

  // Demonstrates how to pipline requests through multiple
  // states.
  Part5::Install(server);
  // }

  // {
  // Begin serving requests.  This is where threads/processes are launched
  // and the above socket begins accepting.
  //
  // The return code indicates the exit status of the server.
  //
  //  0:  Normal case.  This may include cases where a `SIGTERM` was able
  //      to appropriately drain the connections and terminate the
  //      processes.
  // !0:  An error has occurred while attempting to accept connections or
  //      the program has aborted.
  return server.ServeForever();
  // }
}
