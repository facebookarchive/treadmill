# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
bin_PROGRAMS = treadmill_sleep

AM_CPPFLAGS = -I$(top_srcdir)/..

treadmill_sleep_SOURCES = \
  Connection.h \
	Request.h \
  SleepService.h \
  Workload.h \
  Treadmill.cpp

treadmill_sleep_LDADD = \
  $(top_builddir)/libtreadmill.a
