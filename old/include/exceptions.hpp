#ifndef __EXCEPTIONS_HPP
#define __EXCEPTIONS_HPP 

#include <stdexcept>

#define NEW_STANDARD_EXCEPTION(_Name, _Base) \
    class _Name : public _Base { \
    public: \
         template <typename... _Args> \
        _Name(_Args... args) \
            : _Base(args...) \
        {} \
    }; \

namespace fcgi {
    
    ////////////////////////////////////////////////////////////////////////////
    // APPLICATION EXCEPTIONS
    NEW_STANDARD_EXCEPTION(ApplicationException   , std::runtime_error  );
    NEW_STANDARD_EXCEPTION(NullClientException    , ApplicationException);
    NEW_STANDARD_EXCEPTION(UnknownClientException , ApplicationException);
    NEW_STANDARD_EXCEPTION(ClientProtocolException, ApplicationException);
    NEW_STANDARD_EXCEPTION(InvalidManagementQuery , ApplicationException);
    NEW_STANDARD_EXCEPTION(SocketCreationException, std::runtime_error  );
    NEW_STANDARD_EXCEPTION(DomainPermissionIssue  , SocketCreationException);

    
    ////////////////////////////////////////////////////////////////////////////

} // fcgi

#endif // __EXCEPTIONS_HPP
