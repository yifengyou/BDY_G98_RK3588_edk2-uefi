#!/bin/bash

set -xe

cp -a bdy-g98-dts/rk3588-bdy-g98.dtb devicetree/vendor/

./build.sh --clean
./build.sh --device bdy-g98 --release Release


