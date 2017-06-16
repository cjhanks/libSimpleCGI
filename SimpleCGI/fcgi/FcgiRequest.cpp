#include "FcgiRequest.hpp"

#include <cassert>
#include <algorithm>
#include <iomanip>
#include <iostream>

#include "FcgiServer.hpp"
#include "FcgiSocket.hpp"
#include "FcgiStream.hpp"



using std::string;


namespace fcgi {
static constexpr const char* DOCUMENT_URI = "DOCUMENT_URI";
static constexpr const char* QUERY_STRING = "QUERY_STRING";
static constexpr const char* REQUEST_VERB = "REQUEST_METHOD";

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
      // FIXME: Is this necessary?
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

std::string
HttpRequest::GetRouteArgument(const std::string& key, std::string defaultValue )
{
  auto iter = matchingArgs.find(key);
  if (iter == matchingArgs.end())
    return defaultValue;
  else
    return iter->second;
}


size_t
HttpRequest::ContentLength() const
{
  return std::stoul(GetHeader("CONTENT_LENGTH", "0"));
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

InstalledRoute
HttpRequest::GetRoute(MasterServer* master)
{
  return master->Routes().GetRoute(Route(), matchingArgs, httpVerb);
}
} // ns fcgi
