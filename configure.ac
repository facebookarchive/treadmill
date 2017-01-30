# Copyright (c) 2013, Facebook, Inc.
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#   * Neither the name Facebook nor the names of its contributors may be used to
#     endorse or promote products derived from this software without specific
#     prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Download the latest version of autotools
# http://git.savannah.gnu.org/cgit/coreutils.git/tree/scripts/autotools-install
AC_PREREQ([2.69])
AC_INIT([treadmill], [0.1.0], [https://github.com/facebook/treadmill])

AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign dist-bzip2 nostdinc subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CPP
AC_PROG_RANLIB

# Checks for C++14
AX_CXX_COMPILE_STDCXX_14([],[mandatory])

# Thrift compiler
AC_ARG_VAR([THRIFT2_COMP_DIR],
           [Full path to directory containing fbthrift main.py (cpp2 compiler)])
AS_IF([test x"$THRIFT2_COMP_DIR" = x],
      [AC_MSG_ERROR([Path to fbthrift cpp2 compiler must be specified])],
      [THRIFT="python$PYTHON_VERSION $THRIFT2_COMP_DIR/main.py"])

AC_SUBST([THRIFT])

# Disable FBTrace
CXXFLAGS="-DLIBMC_FBTRACE_DISABLE $CXXFLAGS"

# Checks for libraries.
AC_CHECK_LIB([crypto], [MD5_Init], [], [AC_MSG_ERROR(
             [Please install libcrypto])])
AC_CHECK_LIB([double-conversion],[getenv],[],[AC_MSG_ERROR(
             [Please install double-conversion library])])
AC_CHECK_LIB([folly], [getenv], [], [AC_MSG_ERROR(
             [Please install folly library])])
AC_CHECK_LIB([zstd], [getenv], [], [AC_MSG_ERROR(
             [Please install zstd library])])
AC_CHECK_LIB([event], [event_set], [], [AC_MSG_ERROR(
             [Please install libevent])])
AC_CHECK_LIB([gflags], [getenv], [], [AC_MSG_ERROR(
             [Please install google-gflags library])])
AC_CHECK_LIB([glog], [openlog], [], [AC_MSG_ERROR(
             [Please install google-glog library])])
AC_CHECK_LIB([pthread], [pthread_mutex_init], [], [AC_MSG_ERROR(
             [Please install pthread library])])
AC_CHECK_LIB([ssl], [SSL_library_init], [], [AC_MSG_ERROR(
             [Please install openSSL])])
AC_CHECK_LIB([thrift], [main], [], [AC_MSG_ERROR(
             [Please install fbthrift])])
AC_CHECK_LIB([thriftcpp2], [main], [], [AC_MSG_ERROR(
             [Please install fbthrift cpp2])])
AC_CHECK_LIB([thriftprotocol], [main], [], [AC_MSG_ERROR(
             [Please install fbthrift protocol])])
AC_CHECK_LIB([mcrouter], [getenv], [], [AC_MSG_ERROR(
             [Please install mcrouter library])])
AC_CHECK_LIB([numa], [numa_available], [], [AC_MSG_ERROR(
             [Please install numa library])])

# check for boost libs
AX_BOOST_BASE([1.51.0], [], [AC_MSG_ERROR(
              [Please install boost >= 1.51.0])])
AC_CHECK_LIB([boost_context], [main], [], [AC_MSG_ERROR(
             [Please install libboost_context])])
AC_CHECK_LIB([boost_thread], [main], [], [AC_MSG_ERROR(
             [Please install libboost_thread])])
AC_CHECK_LIB([boost_system], [main], [], [AC_MSG_ERROR(
             [Please install libboost_system])])
AC_CHECK_LIB([boost_filesystem], [main], [], [AC_MSG_ERROR(
             [Please install libboost_filesystem])])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_TIME

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strerror])

# Checks for types
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
