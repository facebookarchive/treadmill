################################################
# Makefile for building treadmill              #
#                                              #
# Targets:                                     #
#  - all: Build treadmill                      #
#  - debug: Build debug binary                 #
#  - clean: Remove binary and build files      #
################################################

# folly path
PATH_FOLLY = /path/to/folly/
# double-conversion path
PATH_DBCON = /path/to/double-conversion/

# C++ Compiler
CXX = clang++

# Includes and libraries
INCLUDES   = -I$(PATH_DBCON)src -I$(PATH_FOLLY)
LIBRARIES  = -levent -lfolly -lgflags -lglog -lpthread \
             -L$(PATH_DBCON) -L/usr/local/lib

# Compiler flags
CXX_CFLAGS = -std=c++11 -Wall -D_GNU_SOURCE $(INCLUDES)
CXX_LFLAGS = $(LIBRARIES)
CXX_DFLAGS = -DDEBUG -g

# Source files
SRCS = Connection.cpp \
       Distribution.cpp \
       Histogram.cpp \
       KeyRecord.cpp \
       Request.cpp \
       Statistic.cpp \
       Treadmill.cpp \
       Util.cpp \
       Worker.cpp \
       Workload.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Binary file
BINARY = treadmill

# Build rules
all: $(BINARY)

debug: CXX += $(CXX_DFLAGS)
debug: $(BINARY)

$(BINARY): $(SRCS)
	$(CXX) $(CXX_CFLAGS) $(CXX_LFLAGS) -o $@ $?

clean:
	$(RM) *.o *~ $(BINARY)
