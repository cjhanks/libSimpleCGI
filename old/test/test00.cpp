
#include <memory>
#include "fcgi.hpp"
#include "fastcgi-application.hpp"

using namespace std;
using namespace fcgi;

int
main(int argc, char* argv[])
{
    FastCGI(DomainSocket("/tmp/test.sock")).enterRequestLoop();
}
