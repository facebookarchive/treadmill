ACLOCAL_AMFLAGS = -I m4

noinst_LIBRARIES = libtreadmill.a

libtreadmill_a_SOURCES = \
  Connection.h \
  Histogram.h \
  Request.h \
  Scheduler.h \
  Statistic.h \
  StatisticsManager.h \
  Treadmill.h \
  Util.h \
  Worker.h \
  Workload.h \
  Histogram.cpp \
  Scheduler.cpp \
  Statistic.cpp \
  StatisticsManager.cpp \
  Util.cpp

SUBDIRS = \
	. \
	services/memcached \
	services/libmcrouter \
	services/sleep
