#!/usr/bin/env bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved

source common.sh

[ -d zstd ] || git clone https://github.com/facebook/zstd

cd "zstd/" || "cd fail"

make $MAKE_ARGS && make install PREFIX="$INSTALL_DIR" $MAKE_ARGS
