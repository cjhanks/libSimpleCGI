#ifndef __FCGI_HANDLER_HPP
#define __FCGI_HANDLER_HPP

#include <cstring>
#include <string>
#include <vector>

#include "fcgi-protocol.hpp"
#include "fcgi-http.hpp"
#include "logging.hpp"


namespace fcgi {
class MasterServer;
class LogicalApplicationSocket;

class HttpRequest {
public:
    HttpRequest(const KeyValueMap* httpHeaders,
                const MatchingArgs* matchingArgs,
                const QueryArgument* queryArgs,
                LogicalApplicationSocket* client);

    HttpVerb 
    verb() const { return httpVerb; }
    
    /// {@
    /// All key-value accessors have the behavior of returning an empty string
    /// when the key cannot be found.  No exceptions will be thrown for missing
    /// values.
    
    // Access an HTTP header sent by the upstream FCGI server
    // the query will be converted to all upper case.
    std::string
    getHeader(std::string key, 
              const std::string& defaultValue = "") const;
    
    // Access a query argument from the url:
    // ?query=value
    std::string
    getQueryArgument(const std::string& key);

    // TODO: The URI "anchor" is needed
    //std::string
    //getQueryAnchor();
    
    // When the route was installed it may have had variable arguments
    // eg: /path/<route>/here
    //     /path/example/here
    // Here "route" is the key, "example" is the value
    std::string
    getRouteArgument(const std::string& key);
    /// @}

    ////////////////////////////////////////////////////////////////////////////
    // POST / PUT / PATCH
    ////////////////////////////////////////////////////////////////////////////
    size_t
    contentLength() const;
    
    size_t
    recvAll(std::vector<std::uint8_t>& data);
    
    template <typename _Tp>
    size_t
    recv(_Tp* data, size_t count) {
        return recv(reinterpret_cast<std::uint8_t*>(data), 
                    count * sizeof(_Tp));
    }

    size_t
    recv(std::uint8_t* data, size_t len);
    
    ////////////////////////////////////////////////////////////////////////////
    // DELETE
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // DEBUG 
    ////////////////////////////////////////////////////////////////////////////
    
    void
    dumpRequestDebugTo(std::ostream& strm);

private:
    const KeyValueMap* httpHeaders;
    const MatchingArgs* matchingArgs;
    const QueryArgument* queryArgs;
    HttpVerb httpVerb;
    LogicalApplicationSocket* client;
    
    // 
    size_t bytesRead;
};
    
////////////////////////////////////////////////////////////////////////////////
class HttpHeader {
public:
    HttpHeader(const size_t& responseCode, const std::string& contentType);

    void
    addHeader(const std::string& key, const std::string& val);

    operator std::string() const;
private:
    const size_t responseCode;
    const std::string contentType;
    const std::string responseString;
    std::map<std::string, std::string> headers;
};

class HttpResponse {
public:
    HttpResponse(LogicalApplicationSocket* client);
    ~HttpResponse();
    
    void 
    setResponse(const HttpHeader& header);

    size_t 
    write(const std::string& data);
    
    template <typename _Tp>
    size_t
    write(const _Tp* data, size_t count) {
        return write(reinterpret_cast<const std::uint8_t*>(data), 
                     count * sizeof(_Tp));
    }

    size_t 
    write(const std::uint8_t* data, size_t len);
    
    void
    close();

private:
    LogicalApplicationSocket* client;
};

/**
 */
void
applicationHandler(MasterServer* master, LogicalApplicationSocket* client);
} // ns fcgi

#endif //__FCGI_HANDLER_HPP
