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
    static void
    SetLogLevel(const int& newLevel);

    LOG(const LogLevel& level);
    ~LOG();

    template <typename _Tp>
    inline LOG&
    operator<<(const _Tp& data)
    {
        if (level >= MaximumLogLevel) {
            std::cerr << data;
        }
        return *this;
    }

private:
    static int MaximumLogLevel;
    const int level;

    void 
    printHeader();
};
} // ns fcgi

#endif //__LOGGING_HPP
