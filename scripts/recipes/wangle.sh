#!/usr/bin/env bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved

source common.sh

[ -d wangle ] || git clone https://github.com/facebook/wangle

cd "wangle/wangle/" || die "cd fail"

LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH" \
  LD_RUN_PATH="$INSTALL_DIR/lib:$LD_RUN_PATH" \
  LDFLAGS="-L$INSTALL_DIR/lib $LDFLAGS" \
  CPPFLAGS="-I$INSTALL_DIR/include $CPPFLAGS" \
  cmake -DCMAKE_INSTALL_PREFIX:PATH="$INSTALL_DIR" -DBUILD_TESTS=OFF .
make $MAKE_ARGS && make install $MAKE_ARGS
