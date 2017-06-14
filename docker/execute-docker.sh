#!/bin/bash
# vim: ts=2 sw=2 et ai tw=80
# ---------------------------------------------------------------------------- #
# Runs through all of the different target platforms.
# ---------------------------------------------------------------------------- #

set -e

if [ ! -e docker ]
then
  echo "Run this from the root of the repo"
  exit 1
fi

PLATFORMS=(
  ubuntu_14_04
  ubuntu_16_04
)

for platform in ${PLATFORMS[@]}
do
  docker build -f docker/Docker.${platform} ./
done
