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

# Source files for treadmill framework
TREADMILL_SRCS = Connection.cpp \
								 Distribution.cpp \
								 Histogram.cpp \
								 KeyRecord.cpp \
								 Request.cpp \
								 Statistic.cpp \
								 Treadmill.cpp \
								 Util.cpp \
								 Worker.cpp \
								 Workload.cpp

# Source files for supported workloads
WORKLOAD_SRCS = MemcachedRequest.cpp

# Source files all together
SRCS = $(TREADMILL_SRCS) \
			 $(WORKLOAD_SRCS)

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
