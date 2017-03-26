#include "fcgi.hpp"

using std::string;

namespace fcgi {
string
versionToString(const FastWEB& version)
{
    switch (version) {
        case FastWEB::Version_0_1:
            return "v0.1-alpha";

        case FastWEB::Version_0_2:
            return "v0.2";

        default:
            return "vUnknown";
    }

    return "vUnknown";
}
} // ns fcgi
