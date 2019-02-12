#!/usr/bin/env bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved

source common.sh

cd "$SCRIPT_DIR/../.." || die "cd fail"

THRIFT2_COMPILER=$(readlink -f \
  "$(find "$INSTALL_AUX_DIR" | grep -m 1 'thrift_compiler/main\.py$')")

test "x$THRIFT2_COMPILER" != "x" || die "Couldn't find fbthrift cpp2 compiler"

THRIFT2_COMP_DIR="$(dirname "$THRIFT2_COMPILER")"

autoreconf --install
LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH" \
  LD_RUN_PATH="$INSTALL_DIR/lib:$LD_RUN_PATH" \
  LDFLAGS="-L$INSTALL_DIR/lib $LDFLAGS" \
  CPPFLAGS="-I$INSTALL_DIR/include -I$PKG_DIR/mcrouter $CPPFLAGS" \
  ./configure --prefix="$INSTALL_DIR" THRIFT2_COMP_DIR="$THRIFT2_COMP_DIR" && \
  make $MAKE_ARGS && make install $MAKE_ARGS
