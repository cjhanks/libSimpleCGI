#ifndef __FCGI_HANDLER_HPP
#define __FCGI_HANDLER_HPP

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "FcgiProtocol.hpp"
#include "FcgiHttp.hpp"
#include "FcgiStream.hpp"
#include "FcgiMimetype.hpp"
#include "SimpleCGI/common/Logging.hpp"
#include "bits/FcgiHandler.hpp"


namespace fcgi {
class HttpRequest;
class MasterServer;
class LogicalApplicationSocket;



} // ns fcgi

#endif //__FCGI_HANDLER_HPP
