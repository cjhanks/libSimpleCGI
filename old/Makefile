CC = g++
AR = ar
CXXFLAGS = -O0 -std=c++11 -fPIC
INCLUDES = -Iinclude
LIB      = 
LIBFLAGS = 

SOURCES = src/fastcgi-application.cpp 	\
		  src/fcgi-responder.cpp		\
		  src/network-parsing.cpp		\
		  src/synchronous-request.cpp	

EXAMPLE = test/test00.cpp

OBJECTS = $(SOURCES:.cpp=.o)

LIBSIMPLECGI_SHARED  = libSimpleCGI.so
LIBSIMPLECGI_STATIC  = libSimpleCGI.a
LIBSIMPLECGI_EXAMPLE = cgiexample

.PHONY: clean

all:	$(LIBSIMPLECGI_SHARED) $(LIBSIMPLECGI_STATIC) $(LIBSIMPLECGI_EXAMPLE)
		@echo Compiling libSimpleCGI.so
		@echo Compiling libSimpleCGI.a


$(LIBSIMPLECGI_SHARED): $(OBJECTS)
		$(CC) $(CXXFLAGS) -shared $(INCLUDES) -o $(LIBSIMPLECGI_SHARED) $(OBJECTS) $(LIBFLAGS) $(LIB)

$(LIBSIMPLECGI_STATIC): $(OBJECTS)
		$(AR) rcs $(LIBSIMPLECGI_STATIC) $(OBJECTS)

$(LIBSIMPLECGI_EXAMPLE): $(EXAMPLE)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(LIBSIMPLECGI_EXAMPLE) $(EXAMPLE) $(LIBSIMPLECGI_STATIC) $(LIBFLAGS) $(LIB)
		

.cpp.o:
		$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
		$(RM) *.o src/*.o test/*.o *~ $(LIBSIMPLECGI_SHARED) $(LIBSIMPLECGI_STATIC) $(LIBSIMPLECGI_EXAMPLE)

