# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/..

# Compile the thrift files first
BUILT_SOURCES = \
	services/sleep/gen-cpp2/Sleep.h

services/sleep/gen-cpp2/Sleep.h: services/sleep/sleep.thrift
	PYTHONPATH=$(THRIFT2_COMP_DIR)/..:$$PYTHONPATH $(THRIFT) --gen cpp2 -o services/sleep/ $<

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
	Treadmill.cpp \
	ContinuousStatistic.cpp \
	CounterStatistic.cpp \
	StatisticsManager.cpp \
	Util.cpp

bin_PROGRAMS = \
	treadmill_memcached \
	treadmill_sleep

# Ignore treadmill_libmcrouter for now

treadmill_memcached_SOURCES = \
	services/memcached/Connection.h \
	services/memcached/MemcachedService.h \
	services/memcached/Request.h \
	services/memcached/Treadmill.cpp \
	services/memcached/Workload.h

treadmill_memcached_LDADD = \
	libtreadmill.a

treadmill_sleep_SOURCES = \
	services/sleep/Treadmill.cpp \
	services/sleep/gen-cpp2/sleep_constants.cpp \
	services/sleep/gen-cpp2/sleep_data.cpp \
	services/sleep/gen-cpp2/sleep_types.cpp \
	services/sleep/gen-cpp2/Sleep.cpp \
	services/sleep/gen-cpp2/SleepAsyncClient.cpp \
	services/sleep/gen-cpp2/Sleep_processmap_binary.cpp \
	services/sleep/gen-cpp2/Sleep_processmap_compact.cpp

treadmill_sleep_LDADD = \
	libtreadmill.a

treadmill_libmcrouter_SOURCES = \
	services/libmcrouter/Connection.h \
	services/libmcrouter/LibmcrouterService.h \
	services/libmcrouter/Workload.h \
	services/libmcrouter/Treadmill.cpp

treadmill_libmcrouter_LDADD = \
	libtreadmill.a
