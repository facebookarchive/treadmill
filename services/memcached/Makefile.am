# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
bin_PROGRAMS = treadmill_memcached

AM_CPPFLAGS = -I$(top_srcdir)/..

treadmill_memcached_SOURCES = \
  Connection.h \
  MemcachedService.h \
  Request.h \
  Workload.h \
  Treadmill.cpp

treadmill_memcached_LDADD = \
  $(top_builddir)/libtreadmill.a
