CC = g++
AR = ar
CXXFLAGS = -O3 -DNDEBUG -std=c++11 -fPIC -Wall -Wextra -Werror -pedantic -pthread -g -fsanitize-recover -fstack-protector -fsanitize=address -fsanitize=undefined
INCLUDES = -Iinclude
LIB      =
LIBFLAGS =

SOURCES = src/fcgi-http.cpp \
		  src/fcgi-socket.cpp \
		  src/fcgi-protocol.cpp \
		  src/fcgi-handler.cpp \
		  src/fcgi-server.cpp \
		  src/fcgi.cpp \
		  src/fcgi-io.cpp \
		  src/logging.cpp \
		  src/server/fcgi-pre-fork.cpp \
		  src/server/fcgi-synchronous.cpp \
		  src/server/fcgi-threaded.cpp \


# - UNIT TEST & EXAMPLES --------------------------------------------------------------------------------------------- #
EXAMPLE_00_SRC = test/test00.cpp
EXAMPLE_00_BIN = example_00
EXAMPLE_01_SRC = test/test01.cpp
EXAMPLE_01_BIN = example_01
EXAMPLE_02_SRC = test/test02.cpp
EXAMPLE_02_BIN = example_02
EXAMPLE_03_SRC = test/test03.cpp
EXAMPLE_03_BIN = example_03
EXAMPLE_04_SRC = test/test04.cpp
EXAMPLE_04_BIN = example_04
EXAMPLE_05_SRC = test/test05.cpp
EXAMPLE_05_BIN = example_05

EXAMPLE_BINS = $(EXAMPLE_00_BIN) $(EXAMPLE_01_BIN) $(EXAMPLE_02_BIN) $(EXAMPLE_03_BIN) $(EXAMPLE_04_BIN) $(EXAMPLE_05_BIN)
EXAMPLE_SRCS = $(EXAMPLE_00_SRC) $(EXAMPLE_01_SRC) $(EXAMPLE_02_SRC) $(EXAMPLE_03_SRC) $(EXAMPLE_04_SRC) $(EXAMPLE_05_SRC)

# - LIBRARIES ------------------------------------------------------------------------------------------------------- #
OBJECTS = $(SOURCES:.cpp=.o)

LIBSIMPLECGI_SHARED  = libSimpleCGI.so
LIBSIMPLECGI_STATIC  = libSimpleCGI.a
LIBSIMPLECGI_EXAMPLE = cgiexample

.PHONY: clean mimetype

all:	$(LIBSIMPLECGI_SHARED) $(LIBSIMPLECGI_STATIC) $(LIBSIMPLECGI_EXAMPLE)
		@echo Compiling libSimpleCGI.so
		@echo Compiling libSimpleCGI.a


$(LIBSIMPLECGI_SHARED): $(OBJECTS)
		$(CC) $(CXXFLAGS) -shared $(INCLUDES) -o $(LIBSIMPLECGI_SHARED) $(OBJECTS) $(LIBFLAGS) $(LIB)

$(LIBSIMPLECGI_STATIC): $(OBJECTS)
		$(AR) rcs $(LIBSIMPLECGI_STATIC) $(OBJECTS)

$(EXAMPLE_00_BIN): $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(EXAMPLE_00_BIN) $(EXAMPLE_00_SRC) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI

$(EXAMPLE_01_BIN): $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(EXAMPLE_01_BIN) $(EXAMPLE_01_SRC) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI

$(EXAMPLE_02_BIN): $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(EXAMPLE_02_BIN) $(EXAMPLE_02_SRC) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI

$(EXAMPLE_03_BIN): $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(EXAMPLE_03_BIN) $(EXAMPLE_03_SRC) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI

$(EXAMPLE_04_BIN): $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(EXAMPLE_04_BIN) $(EXAMPLE_04_SRC) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI

$(EXAMPLE_05_BIN): $(LIBSIMPLECGI_SHARED)
		$(CC) $(CXXFLAGS) $(INCLUDES) -o $(EXAMPLE_05_BIN) $(EXAMPLE_05_SRC) $(LIBFLAGS) $(LIB) -L. -lSimpleCGI

$(LIBSIMPLECGI_EXAMPLE): $(EXAMPLE_00_BIN) $(EXAMPLE_01_BIN) $(EXAMPLE_02_BIN) $(EXAMPLE_03_BIN) $(EXAMPLE_04_BIN) $(EXAMPLE_05_BIN)

.cpp.o:
		$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

mimetype:
		@echo Generating mimetype file
		@echo $(shell ./tool/mimetype/generate \
					--input-csv ./tool/mimetype/mimetype.csv \
					--input-template ./tool/mimetype/template.jinja \
					--output-header ./include/fcgi-mimetype.hpp)

clean:
		$(RM) *~ $(LIBSIMPLECGI_SHARED) $(LIBSIMPLECGI_STATIC) $(LIBSIMPLECGI_EXAMPLE) $(OBJECTS) $(EXAMPLE_BINS)

