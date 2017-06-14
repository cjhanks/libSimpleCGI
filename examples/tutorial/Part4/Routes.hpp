#if WITH_WSGI == 1
#ifndef PART4_ROUTES_HPP_
#define PART4_ROUTES_HPP_

#include <SimpleCGI/SimpleCGI.hpp>


namespace Part4 {
void
Install(fcgi::ServerConfig& config);
} // ns Part4

#endif // PART4_ROUTES_HPP_
#endif // WITH_WSGI == 1
