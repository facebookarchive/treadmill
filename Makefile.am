ACLOCAL_AMFLAGS = -I m4

noinst_LIBRARIES = libtreadmill.a

libtreadmill_a_SOURCES = \
  Connection.h \
  Histogram.h \
  Request.h \
  RandomEngine.h \
  Scheduler.h \
  Statistic.h \
  ContinuousStatistic.h \
  CounterStatistic.h \
  StatisticsManager.h \
  Treadmill.h \
  Util.h \
  Worker.h \
  Workload.h \
  Histogram.cpp \
  RandomEngine.cpp \
  Scheduler.cpp \
  ContinuousStatistic.cpp \
  CounterStatistic.cpp \
  StatisticsManager.cpp \
  Util.cpp

SUBDIRS = \
	. \
	services/memcached \
	services/libmcrouter \
	services/sleep
