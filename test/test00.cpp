
#include <memory>
#include "fcgi.hpp"
#include "synchronous-application.hpp"

using namespace std;
using namespace fcgi;

int
main(int argc, char* argv[])
{
    SynchronousApplication app(DomainSocket("/tmp/test.sock"));

    while (true) {
        unique_ptr<SynchronousRequest> req(app.acceptRequest());
    }
}
