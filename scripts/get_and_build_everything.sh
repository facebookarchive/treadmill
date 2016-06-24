#!/usr/bin/env bash

set -ex

ORDER=$1

PKG_DIR=$2/pkgs
INSTALL_DIR=$2/install

SERVICE=$3

mkdir -p "$PKG_DIR" "$INSTALL_DIR"

cd $(dirname $0)

for script in $(ls "order_$ORDER/" | egrep '^[0-9]+_.*[^~]$' | sort -n); do
  script_order=${script:0:2}
  if [[ "$script_order" == "20"  && "$SERVICE" != "all" ]]; then
    if [[ "$SERVICE" == "memcached" && "$script" == "20_mcrouter" ]] ||
       [[ "$SERVICE" == "libmcrouter" && "$script" == "20_mcrouter" ]] ||
       [[ "$SERVICE" == "sleep" && "$script" == "20_fbthrift" ]]; then
      "./order_$ORDER/$script" "$PKG_DIR" "$INSTALL_DIR" "$SERVICE" "$MAKE_ARGS"
    fi
  else
    "./order_$ORDER/$script" "$PKG_DIR" "$INSTALL_DIR" "$SERVICE" "$MAKE_ARGS"
  fi
done

printf "%s\n" "Treadmill installed in $INSTALL_DIR/"
