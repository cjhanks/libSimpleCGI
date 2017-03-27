#include "FcgiHandler.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "FcgiServer.hpp"
#include "FcgiSocket.hpp"
#include "FcgiHttp.hpp"
#include "FcgiStream.hpp"
#include "SimpleCGI/common/Logging.hpp"


namespace fcgi {
using std::string;
using std::stringstream;
using std::vector;


static constexpr const char* DOCUMENT_URI = "DOCUMENT_URI";
static constexpr const char* QUERY_STRING = "QUERY_STRING";
static constexpr const char* REQUEST_VERB = "REQUEST_METHOD";

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

void
HttpRequest::DumpRequestDebugTo(std::ostream& strm)
{
  using std::setw;
  using std::endl;

  strm << setw(4)
     << VerbToVerbString(httpVerb)
     << ":   "
     << httpHeaders.at(DOCUMENT_URI)
     << " ["
     << ContentLength()
     << "]"
     << endl;

  strm << "  FCGI Headers:"
     << endl;
  for (auto& ref: httpHeaders) {
    strm << "  "
       << setw(24)
       << ref.first
       << " = "
       << ref.second
       << endl;
  }


  strm << "  Matching Arguments:"
     << endl;

  for (auto& ref: matchingArgs) {
    strm << "  "
       << setw(12)
       << ref.first
       << " = "
       << ref.second
       << endl;
  }

  strm << "  Query Arguments:"
     << endl;

  for (auto& ref: queryArgs.queryArgs) {
    strm << "  "
       << setw(12)
       << ref.first
       << " = "
       << ref.second
       << endl;
  }

  strm << endl;
}

string
HttpRequest::GetHeader(string key, const string& defaultValue) const
{
  static auto cleanHeader = [](const char c) -> char {
    switch (c) {
      case '-':
        return '_';

      default:
        return ::toupper(c);
    }
  };

  std::transform(key.begin(), key.end(), key.begin(), cleanHeader);

  auto it = httpHeaders.find(key);
  if (it == httpHeaders.end() || it->second.size() == 0) {
    return defaultValue;
  } else {
    return it->second;
  }
}

size_t
HttpRequest::ContentLength() const
{
  return std::stoul(GetHeader("CONTENT_LENGTH", "0"));
}

size_t
HttpRequest::RecvAll(vector<uint8_t>& data)
{
  size_t contentSize = ContentLength();
  if (contentSize == bytesRead) {
    return 0;
  } else {
    data.resize(contentSize);
    size_t sizeRead = client->ReadData(data.data(), contentSize);
    if (sizeRead > 0) {
      bytesRead += sizeRead;
    }
    return sizeRead;
  }
}

size_t
HttpRequest::RecvAll(std::string& data)
{
  vector<uint8_t> buffer;
  auto size = RecvAll(buffer);
  data.assign(buffer.begin(), buffer.end());
  return size;
}

Istream
HttpRequest::ToStream()
{
  return Istream(this);
}

size_t
HttpRequest::recv(uint8_t* data, size_t len)
{
  size_t contentSize = ContentLength();
  if (contentSize == bytesRead) {
    return 0;
  } else {
    size_t newBytes =
      client->ReadData(data, std::min(contentSize - bytesRead, len));
    if (newBytes > 0) {
      bytesRead += newBytes;
    }

    return newBytes;
  }
}

HttpRequest::HttpRequest(LogicalApplicationSocket* client)
  : httpVerb(HttpVerb::UNDEFINED), client(client), bytesRead(0)
{
  // HTTP Headers
  Header header(client->GetHeader());
  while (header.type == HeaderType::PARAMS) {
    client->mergeKeyValueMap(header, httpHeaders);
    header = client->GetHeader();
  }

  // Route information
  auto RouteIter = httpHeaders.find(DOCUMENT_URI);
  if (RouteIter == httpHeaders.end()) {
    assert(1 == 0);
  } else {
    httpRoute = RouteIter->second;
  }

  // Query Arguments
  auto queryIter = httpHeaders.find(QUERY_STRING);
  if (queryIter != httpHeaders.end()) {
    queryArgs = QueryArgument::fromRawString(queryIter->second);
  }

  // Find the Verb
  auto VerbStr = httpHeaders.find(REQUEST_VERB);
  if (VerbStr == httpHeaders.end()) {
    assert(1 == 0);
  } else {
    httpVerb = VerbStringToVerb(VerbStr->second);
  }
}

Maybe
HttpRequest::GetRoute(MasterServer* master)
{
  return master->Routes().GetRoute(Route(), matchingArgs, httpVerb);
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

HttpHeader::HttpHeader(const size_t& responseCode)
  : HttpHeader(responseCode, statusCodeToString(responseCode))
{}

HttpHeader::HttpHeader(
    const size_t& responseCode, const std::string& responseString)
  : responseCode(responseCode)
  , responseString(responseString)
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
  client->ExitCode(ProtocolStatus::REQUEST_COMPLETE);
}

void
HttpResponse::LogError(const string& message)
{
  client->LogError(
      reinterpret_cast<const uint8_t*>(message.c_str()),
      message.size());
}

void
HttpResponse::SetResponse(const HttpHeader& header)
{
  write(static_cast<string>(header));
}

Ostream
HttpResponse::ToStream()
{
  return Ostream(this);
}

size_t
HttpResponse::write(const string& data)
{
  return write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

size_t
HttpResponse::write(const uint8_t* data, size_t len)
{
  return client->SendData(data, len);
}

void
HttpResponse::Close()
{}

} // ns fcgi
