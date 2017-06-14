#!/bin/bash
# vim: ts=2 sw=2 et ai tw=80
# ---------------------------------------------------------------------------- #
# Runs through all of the necessary permutations of builds to ensure they are
# building correctly.
# ---------------------------------------------------------------------------- #
set -e

JCOUNT=4


run_builds() {
  args="${*}"
  rm -rf build

  targets=("release" "debug" "native")
  for target in ${targets[@]}
  do
    scons -j${JCOUNT} --mode=${target} ${args}
  done
}

run() {
  run_builds
  run_builds --wsgi
}

export CXX=g++
run

export CXX=clang++
run
