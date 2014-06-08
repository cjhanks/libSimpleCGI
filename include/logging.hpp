#ifndef __LOGGING_HPP
#define __LOGGING_HPP

#include <iomanip>
#include <iostream>

namespace fcgi {

enum LogLevel {
    DEBUG   = 2,
    INFO    = 4,
    WARNING = 6,
    ERROR   = 8
};

class LOG {
public:
    LOG(const LogLevel& level)
        : level(static_cast<int>(level))
    {}

    virtual ~LOG() {
        std::cerr << std::endl;
    }

    template <typename _Tp>
    std::ostream& operator<<(_Tp& data)
    {
        printHeader();
        std::cerr << data;
        return std::cerr;
    }

private:
    const int level;

    inline void printHeader() 
    {
        std::cerr << "[" << time(0x0) << "]"
                  << " ("
                  << level
                  << ") ";
    }
};
} // ns fcgi

#endif //__LOGGING_HPP
