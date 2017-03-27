#ifndef __FCGI_HPP
#define __FCGI_HPP

#include <string>

namespace fcgi {
enum class FastWEB {
  Version_0_1   = 0x0001,
  Version_0_2   = 0x0002,
};

static constexpr FastWEB CurrentVersion = FastWEB::Version_0_2;

std::string
VersionToString(const FastWEB& version);
} // ns fcgi

////////////////////////////////////////////////////////////////////////////////

#include "SimpleCGI/fcgi/FcgiServer.hpp"
#include "SimpleCGI/fcgi/FcgiSocket.hpp"

#endif //__FCGI_HPP
