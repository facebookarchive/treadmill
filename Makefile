# Include directories
I = -I ../ -I $(GTEST_INCLUDE_DIR)

# Compiler flags
CFLAGS = -Wall -levent -pthread -D_GNU_SOURCE $(I)
DFLAGS = -g

SRC = Treadmill.cpp\
      Connection.cpp\
      Request.cpp\
      Util.cpp\
      Worker.cpp\

treadmill: $(SRC)
	 clang++ -std=c++11 -stdlib=libc++ -L/usr/local/lib -lglog -lgflags -o treadmill $(SRC)
