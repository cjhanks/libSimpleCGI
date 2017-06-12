#include "FcgiHandler.hpp"
#include "../FcgiResponse.hpp"
#include "../FcgiRequest.hpp"
#include "SimpleCGI/common/Logging.hpp"


namespace fcgi { namespace bits {

////////////////////////////////////////////////////////////////////////////////

HttpRequestStreamBuf::HttpRequestStreamBuf(HttpRequest* request)
  : request(request)
{}

std::streambuf::int_type
HttpRequestStreamBuf::underflow()
{
  if (gptr() == egptr()) {
  size_t size = request->recv(buffer.data(), buffer.size());
  setg(buffer.data(),
     buffer.data(),
     buffer.data() + size);
  }

  return gptr() == egptr()
     ? std::char_traits<char>::eof()
     : std::char_traits<char>::to_int_type(*gptr());
}

////////////////////////////////////////////////////////////////////////////////

HttpResponseStreamBuf::HttpResponseStreamBuf(HttpResponse* response)
  : response(response)
{
  setp(buffer.data(), buffer.data() + buffer.size());
}

HttpResponseStreamBuf::~HttpResponseStreamBuf()
{
  sync();
}

std::streambuf::int_type
HttpResponseStreamBuf::overflow(std::streambuf::int_type ch)
{
  auto write = pptr() - pbase();
  if (write) {
  int written = response->write(buffer.data(), write);
  if (write != written)
    return traits_type::eof();
  }

  setp(buffer.data(), buffer.data() + buffer.size());
  if (!traits_type::eq_int_type(ch, traits_type::eof()))
  sputc(ch);

  return traits_type::not_eof(ch);
}

int
HttpResponseStreamBuf::sync()
{
  auto result = overflow(traits_type::eof());
  // TODO:
  //response->flush();
  return traits_type::eq_int_type(result, traits_type::eof())
     ? -1
     : +0;
}
} // ns bits
} // ns fcgi
