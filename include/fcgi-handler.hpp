#ifndef __FCGI_HANDLER_HPP
#define __FCGI_HANDLER_HPP

#include <cstring>
#include <string>
#include <vector>

#include "fcgi-protocol.hpp"
#include "fcgi-http.hpp"
#include "fcgi-mimetype.hpp"
#include "logging.hpp"


namespace fcgi {
class MasterServer;
class LogicalApplicationSocket;

class HttpRequest {
public:
    HttpRequest(LogicalApplicationSocket* client);

    HttpVerb 
    verb() const { return httpVerb; }

    std::string
    route() const { return httpRoute; }
    
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

    // When the route was installed it may have had variable arguments
    // eg: /path/<route>/here
    //     /path/example/here
    // Here "route" is the key, "example" is the value
    std::string
    getRouteArgument(const std::string& key);
    /// @}
    
    // NOTE: Currently this is not a "cheap" function, as it performs string parsing on
    // every operation.
    size_t
    contentLength() const;

    ////////////////////////////////////////////////////////////////////////////
    // POST / PUT / PATCH
    ////////////////////////////////////////////////////////////////////////////
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
    // DEBUG & INTERNAL
    ////////////////////////////////////////////////////////////////////////////
    Maybe
    getRoute(MasterServer* master);

    void
    dumpRequestDebugTo(std::ostream& strm);

private:
    std::string httpRoute;
    KeyValueMap httpHeaders;
    MatchingArgs matchingArgs;
    QueryArgument queryArgs;
    HttpVerb httpVerb;
    LogicalApplicationSocket* client;
    
    // 
    size_t bytesRead;
};
    
////////////////////////////////////////////////////////////////////////////////
class HttpHeader {
public:
    HttpHeader(const size_t& responseCode, const std::string& contentType);
    HttpHeader(const size_t& responseCode, const MimeType& mimeType)
        : HttpHeader(responseCode, mimeTypeToString(mimeType))
    {}

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
    logError(const std::string& message);

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
