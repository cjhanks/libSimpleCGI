#include "logging.hpp"

namespace fcgi {
int
LOG::MaximumLogLevel = LogLevel::WARNING;

void
LOG::SetLogLevel(const int& newLevel)
{
    LOG::MaximumLogLevel = newLevel;
}

LOG::LOG(const LogLevel& level)
    : level(static_cast<int>(level))
{
    printHeader();
}

LOG::~LOG()
{
    if (level >= MaximumLogLevel) {
        std::cerr << std::endl;
    }
}

void
LOG::printHeader()
{
    if (level >= MaximumLogLevel) {
        std::cerr << "[" << time(0x0) << "]" << " (" << level << ") ";
    }
}
} // ns fcgi
