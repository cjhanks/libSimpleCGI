
#include "fcgi-protocol.hpp"
#include <cassert>
#include "logging.hpp"

using std::string;

namespace fcgi {
/**
 * This function assumes input data is sane according to the FCGI protocol, it
 * can segfault onn bad input
 *
 * @return true
 *      If data input perfectly aligned with the key-value pairs.
 */
bool
readIntoKeyValueMap(const uint8_t* data, size_t len,
                    KeyValueMap& keyValueMap)
{
    const uint8_t* begin = data;
    const uint8_t* end   = data + len;

    static auto parseLength = [&]() -> uint32_t {
        if (1 != (*begin) >> 7) {
            assert(begin + 1 <= end);
            return *(begin++);
        } else {
            assert(begin + 4 <= end);
            return ((*(begin++) & 0x7f) << 24)
                 + ((*(begin++)       ) << 16)
                 + ((*(begin++)       ) << 8 )
                 + ((*(begin++)       ) << 0 );
        }
    };

    while (begin < end) {
        // parse name length
        size_t keyLength = parseLength();
        size_t valLength = parseLength();
    
        assert(begin + keyLength + valLength <= end);
        const string key(begin, 
                         begin + keyLength);
        const string val(begin + keyLength,
                         begin + keyLength + valLength);
        
        keyValueMap[key] = val;
        begin += (keyLength + valLength);
    }

    return begin == end;
}
} // ns fcgi
