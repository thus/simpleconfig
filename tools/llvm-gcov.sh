#!/bin/bash
#
# llvm-gcov.sh - Wrapper script used by LCOV to generate coverage when
#                compiling with clang.
#
# Example usage:
#   lcov --directory ./src --gcov-tool llvm-gcov.sh --capture -o cov.info

CLANG_BIN_DIR=$(dirname $(readlink -f /usr/bin/clang))

exec ${CLANG_BIN_DIR}/llvm-cov gcov "$@"
