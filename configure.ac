# Copyright (c) 2016, Facebook, Inc.
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

AM_INIT_AUTOMAKE([foreign dist-bzip2 nostdinc])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CPP
AC_PROG_RANLIB

# Checks for C++11
AX_CXX_COMPILE_STDCXX_11([],[mandatory])

# Checks for libraries.
AC_CHECK_LIB([glog], [openlog], [], [AC_MSG_ERROR(
             [Please install google-glog library])])
AC_CHECK_LIB([gflags], [getenv], [], [AC_MSG_ERROR(
             [Please install google-gflags library])])
AC_CHECK_LIB([folly], [getenv], [], [AC_MSG_ERROR(
             [Please install folly library])])
AC_CHECK_LIB([event], [event_set], [], [AC_MSG_ERROR(
             [Please install libevent])])

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

AC_CONFIG_FILES([Makefile
                 services/memcached/Makefile
                 services/libmcrouter/Makefile
                 services/sleep/Makefile])

AC_OUTPUT
