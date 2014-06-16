#include "fcgi-responder.hpp"
#include "logging.hpp"
#include "exceptions.hpp"
#include <iostream>

namespace fcgi {
FCGI_Responder::FCGI_Responder(const Header& header, const Begin beginMessage)
    : version(header.version), requestId(header.requestId), beginMessage(beginMessage)
{}

void
FCGI_Responder::operator()(UniqueSocket& socket, VectorBuffer& buffer)
{
    Header header(socket.readHeader());
    assertIsSecureHeader(header);

    // get http headers & params from upstream server
    while (HeaderType::PARAMS == header.type) {
        socket.readIntoBuffer(header, buffer);
        readBufferIntoKeyValuePair(buffer, httpParams); 
        header = socket.readHeader();
        assertIsSecureHeader(header);
    }

}

void
FCGI_Responder::assertIsSecureHeader(const Header& header)
{
    if (header.requestId != requestId || header.version != version) {
        // TODO: Throw exception
    }
}

} // ns fcgi
