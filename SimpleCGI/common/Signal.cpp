#include "Signal.hpp"

#include <execinfo.h>
#include <unistd.h>

#include <cstdio>
#include <csignal>
#include <cstdlib>



namespace fcgi { namespace bits {
namespace {
void StackTraceHandler(int sig)
{
  static constexpr size_t Size = 10;

  void* array[Size];
  size_t size = backtrace(array, Size);

  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
} // ns

void
InstallSignalHandler()
{
  std::signal(SIGINT, StackTraceHandler);
  std::signal(SIGABRT, StackTraceHandler);
}
} // ns bits
} // ns fcgi
