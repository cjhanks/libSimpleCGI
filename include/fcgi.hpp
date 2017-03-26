#ifndef __FCGI_HPP
#define __FCGI_HPP

#include <string>

namespace fcgi {
enum class FastWEB {
    Version_0_1     = 1,
    Version_0_2     = 2,
};

static constexpr FastWEB CurrentVersion = FastWEB::Version_0_2;

std::string
versionToString(const FastWEB& version);

} // ns fcgi

#endif //__FCGI_HPP
