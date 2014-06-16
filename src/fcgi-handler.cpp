#include "fcgi-handler.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "fcgi-server.hpp"
#include "fcgi-socket.hpp"
#include "fcgi-http.hpp"
#include "logging.hpp"


namespace fcgi {
using std::string;
using std::stringstream;
using std::vector;


static constexpr const char* DOCUMENT_URI = "DOCUMENT_URI";
static constexpr const char* QUERY_STRING = "QUERY_STRING";
static constexpr const char* REQUEST_VERB = "REQUEST_METHOD";

void
applicationHandler(MasterServer* master, LogicalApplicationSocket* client)
{
    KeyValueMap httpHeaders;
    MatchingArgs matchArgs;
    QueryArgument queryArgs;
    Header header(client->getHeader());
    while (header.type == HeaderType::PARAMS) {
        client->mergeKeyValueMap(header, httpHeaders);
        header = client->getHeader();
    }
    
    // -- find the route
    auto routeStr = httpHeaders.find(DOCUMENT_URI);
    if (routeStr == httpHeaders.end()) {
        assert(1 == 0);
    }

    auto maybeRoute = master->HttpRoutes.getRoute(routeStr->second, matchArgs);
    if (!maybeRoute) {
        return;
    } 
    
    // -- find the query args
    auto queryStr = httpHeaders.find(QUERY_STRING);
    if (queryStr != httpHeaders.end()) {
        queryArgs = QueryArgument::fromRawString(queryStr->second);
    }
    
    HttpRequest  req(&httpHeaders, &matchArgs, &queryArgs, client);
    HttpResponse res(client);

    maybeRoute(req, res);
}

void
HttpRequest::dumpRequestDebugTo(std::ostream& strm)
{
    using std::setw;
    using std::endl;

    strm << setw(4)
         << verbToVerbString(httpVerb)
         << ":   "
         << httpHeaders->at(DOCUMENT_URI)
         << " ["
         << contentLength()
         << "]"
         << endl;
    
    strm << "    FCGI Headers:"
         << endl;
    for (auto& ref: *httpHeaders) {
        strm << "    "
             << setw(24)
             << ref.first
             << " = "
             << ref.second
             << endl;
    }


    strm << "    Matching Arguments:"
         << endl;

    for (auto& ref: *matchingArgs) {
        strm << "    "
             << setw(12)
             << ref.first
             << " = "
             << ref.second
             << endl;
    }

    strm << "    Query Arguments:"
         << endl;

    for (auto& ref: queryArgs->queryArgs) {
        strm << "    "
             << setw(12)
             << ref.first
             << " = "
             << ref.second
             << endl;
    }
    
    strm << endl;
}
    
string
HttpRequest::getHeader(string key, const string& defaultValue) const
{
    static auto cleanHeader = [&](const char c) -> char {
        switch (c) {
            case '-':
                return '_';

            default:
                return ::toupper(c);
        }
    };

    std::transform(key.begin(), key.end(), key.begin(), cleanHeader);
                  
    auto it = httpHeaders->find(key);
    if (it == httpHeaders->end() || it->second.size() == 0) {
        return defaultValue;
    } else {
        return it->second;
    }
}

size_t
HttpRequest::contentLength() const
{
    return std::stoul(getHeader("CONTENT_LENGTH", "0"));
}
    
size_t
HttpRequest::recvAll(vector<uint8_t>& data)
{
    size_t contentSize = contentLength();
    if (contentSize == bytesRead) {
        return 0;
    } else {
        data.resize(contentSize);
        size_t sizeRead = client->readData(data.data(), contentSize);
        if (sizeRead > 0) {
            bytesRead += sizeRead;
        }
        return sizeRead;
    }
}

size_t
HttpRequest::recv(uint8_t* data, size_t len)
{
    size_t contentSize = contentLength();
    if (contentSize == bytesRead) {
        return 0;
    } else {
        size_t newBytes = client->readData(data, 
                                           std::min(contentSize - bytesRead,
                                                    len));
        if (newBytes > 0) {
            bytesRead += newBytes;
        }

        return newBytes;
    }
}

HttpRequest::HttpRequest(const KeyValueMap* httpHeaders,
                         const MatchingArgs* matchingArgs,
                         const QueryArgument* queryArgs,
                         LogicalApplicationSocket* client)
    : httpHeaders(httpHeaders), matchingArgs(matchingArgs), 
      queryArgs(queryArgs), httpVerb(HttpVerb::UNDEFINED), 
      client(client), bytesRead(0)
{
    // Find the Verb
    auto verbStr = httpHeaders->find(REQUEST_VERB);
    if (verbStr == httpHeaders->end()) {
        assert(1 == 0);
    } else {
        httpVerb = verbStringToVerb(verbStr->second);
    }

    if (httpVerb == HttpVerb::UNDEFINED) {
        assert(1 == 0);
    }
}
    
////////////////////////////////////////////////////////////////////////////////

namespace {
string
statusCodeToString(size_t code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "CREATED";
        case 202: return "Accepted";
        case 203: return "Partial Information";
        case 204: return "No Response";
        case 400: return "Bad request";
        case 401: return "Unauthorized";
        default:
            return "Undefined";
    }
}
} // ns

HttpHeader::HttpHeader(const size_t& responseCode, const string& contentType)
    : responseCode(responseCode), 
      contentType(contentType),
      responseString(statusCodeToString(responseCode))
{}

void
HttpHeader::addHeader(const string& key, const string& value)
{
    headers[key] = value;
}

HttpHeader::operator string() const
{
    stringstream ss;
    ss << "Status: " << responseCode << " " << responseString
       << "\r\n";
    for (auto& ref: headers) {
        ss << ref.first << ": " << ref.second << "\r\n";
    }
    ss << "\n";
    return ss.str();

}

HttpResponse::HttpResponse(LogicalApplicationSocket* client)
    : client(client)
{}

HttpResponse::~HttpResponse()
{
    client->exitCode(ProtocolStatus::REQUEST_COMPLETE);
}

void
HttpResponse::setResponse(const HttpHeader& header)
{
    write(static_cast<string>(header));
}

size_t
HttpResponse::write(const string& data)
{
    return write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

size_t
HttpResponse::write(const uint8_t* data, size_t len)
{
    return client->sendData(data, len);
}

void
HttpResponse::close()
{}

} // ns fcgi
