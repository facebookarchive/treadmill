################################################
# Makefile for building treadmill              #
#                                              #
# Targets:                                     #
#  - all: Build treadmill                      #
#  - debug: Build debug binary                 #
#  - clean: Remove binary and build files      #
################################################

# C++ Compiler
CXX = clang++

# Compiler flags
CXX_CFLAGS = -std=c++11 -Wall -D_GNU_SOURCE
CXX_LFLAGS =
CXX_DFLAGS = -g
INCLUDES   =
LIBRARIES  = -levent -lpthread -lglog -lgflags -L/usr/local/lib

# Source files
SRCS = Connection.cpp \
       Histogram.cpp \
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

debug: CXX += -DDEBUG -g
debug: $(BINARY)

$(BINARY): $(SRCS)
	$(CXX) $(CXX_CFLAGS) $(CXX_LFLAGS) $(LIBRARIES) -o $@ $?

clean:
	$(RM) *.o *~ $(BINARY)
