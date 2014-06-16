CC = g++
AR = ar
CXXFLAGS = -O3 -std=c++11 -fPIC -fstack-protector-all 
INCLUDES = -Iinclude
LIB      = 
LIBFLAGS = 

SOURCES = src/fcgi-http.cpp \
		  src/fcgi-socket.cpp \
		  src/fcgi-protocol.cpp \
		  src/fcgi-handler.cpp \
		  src/fcgi-server.cpp

EXAMPLE = test/test00.cpp test/base64.c

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

$(LIBSIMPLECGI_EXAMPLE): $(EXAMPLE) $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(LIBSIMPLECGI_EXAMPLE) $(EXAMPLE) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI
		

.cpp.o:
		$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
		$(RM) *.o src/*.o test/*.o *~ $(LIBSIMPLECGI_SHARED) $(LIBSIMPLECGI_STATIC) $(LIBSIMPLECGI_EXAMPLE)

