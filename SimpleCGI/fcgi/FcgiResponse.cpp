#include "FcgiResponse.hpp"

#include <cassert>
#include <sstream>

#include "FcgiServer.hpp"
#include "FcgiSocket.hpp"
#include "FcgiStream.hpp"


using std::string;
using std::stringstream;


namespace fcgi {
namespace {
// FIXME:  Make this user modifiable
string
statusCodeToString(size_t code)
{
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
  : responseCode(responseCode)
  , responseString(statusCodeToString(responseCode))
{}

HttpHeader::HttpHeader(
    const size_t& responseCode, const std::string& contentType)
  : HttpHeader(responseCode)
{
  Add("content-type", contentType);
}

void
HttpHeader::Add(const string& key, const string& value)
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
  // FIXME:  When a socket has a broken pipe error this still tries to send
  //         more IO, which throws another exception.  The result is an
  //         unhandled exception in an exception handler.
  //
  //         This fixes it... but it's very bad style.
  try {
    client->ExitCode(ProtocolStatus::REQUEST_COMPLETE);
  } catch (...) {
    LOG(ERROR) << "Failed to send exit code";
  }
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
  assert(nullptr != data);
  assert(nullptr != client);
  return client->SendData(data, len);
}

void
HttpResponse::Close()
{}

} // ns fcgi
