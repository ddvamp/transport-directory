#!/bin/bash

set -o errexit
set -o nounset
USAGE="Usage: $(basename $0) [--verbose | -v] [debug | release] | clear"

BUILD=./build
VERBOSE=

for arg; do
	case "$arg" in
		--help|-h) echo $USAGE; exit 0;;
		--verbose|-v) VERBOSE='VERBOSE=1';;
		debug) TYPE=Debug; BUILD_DIR=$BUILD/debug;;
		release) TYPE=Release; BUILD_DIR=$BUILD/release;;
		clear) [[ -d $BUILD ]] && rm -rf $BUILD; \
			[[ -d ./bin ]] && rm -rf ./bin; \
			exit 0;;
		*) echo -e "Unknown option $arg\n$USAGE" >&2; exit 1;;
	esac
done

cmake -S . -B $BUILD_DIR --warn-uninitialized -DCMAKE_BUILD_TYPE=$TYPE \
	-DCMAKE_CXX_COMPILER=g++-11
cmake --build $BUILD_DIR --parallel 12 -- --no-print-directory $VERBOSE
