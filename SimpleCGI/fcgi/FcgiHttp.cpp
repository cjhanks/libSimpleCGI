#include "FcgiHttp.hpp"

#include <cassert>
#include <locale>
#include <sstream>
#include "SimpleCGI/common/Logging.hpp"

namespace fcgi {
using std::endl;
using std::getline;
using std::locale;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

namespace {
string
urlDecode(const string& input)
{
  static locale loc;

  static auto toChar = [](const char c) -> char {
    if (c >= '0' && c <= '9') {
      return c - '0';
    } else if (c >= 'A' && c <= 'F') {
      return c - 'A';
    } else {
      // THROW
      return 0;
    }
  };

  stringstream ostrm;
  for (size_t i = 0; i < input.size(); ++i) {
    switch (input[i]) {
      case '%':
        if (i + 2 < input.size()) {
          ostrm << static_cast<char>(toChar(input[i + 1]) * 16
                       + toChar(input[i + 2]));
          i += 2;
        } else {
          // THROW
        }
        break;

      case '+':
        break;

      default:
        if (isalnum(input[i], loc)) {
          ostrm << input[i];
        } else {
          // THROW
        }
        break;
    }
  }

  return ostrm.str();
}
} // ns

HttpVerb
VerbStringToVerb(const string& VerbStr)
{
  if (VerbStr == "GET")  return HttpVerb::GET;
  if (VerbStr == "POST")   return HttpVerb::POST;
  if (VerbStr == "PUT")  return HttpVerb::PUT;
  if (VerbStr == "PATCH")  return HttpVerb::PATCH;
  if (VerbStr == "DELETE") return HttpVerb::DELETE;

  return HttpVerb::UNDEFINED;
}

string
VerbToVerbString(const HttpVerb& Verb)
{
  switch (Verb) {
    case HttpVerb::UNDEFINED:
      return "UNDEFINED";

    case HttpVerb::GET:
      return "GET";

    case HttpVerb::POST:
      return "POST";

    case HttpVerb::PUT:
      return "PUT";

    case HttpVerb::PATCH:
      return "PATCH";

    case HttpVerb::DELETE:
      return "DELETE";

    case HttpVerb::ANY:
      return "ANY";

    default:
      return "UNDEFINED";
  }
}

////////////////////////////////////////////////////////////////////////////////

QueryArgument
QueryArgument::fromRawString(const string& rawString)
{
  static string Separators("&;");
  QueryArgMap queryArgs;

  size_t head = 0;
  size_t tail = 0;
  while (tail != string::npos) {
    tail = rawString.find_first_of(Separators, head);
    string element(rawString.substr(head, tail - head));
    size_t mids = element.find_first_of("=");

    std::string key;
    std::string value;

    if (mids == string::npos) {
      // there is no '=' sign
      key = urlDecode(element);
    } else {
      key = urlDecode(element.substr(0, mids));
      value = urlDecode(element.substr(mids + 1));
    }

    queryArgs[key] = value;
    head = tail + 1;
  }

  return QueryArgument(queryArgs);
}

QueryArgument::QueryArgument(const QueryArgMap& queryArgMap)
  : queryArgs(queryArgMap)
{}

string
QueryArgument::GetArgument(const string& key, const string& defaultValue) const
{
  auto it = queryArgs.find(key);
  if (it == queryArgs.end()) {
    return defaultValue;
  } else {
    return it->second;
  }
}


} // ns fcgi
