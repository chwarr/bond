#!/bin/zsh

set -eux

# Target version of CMake to install.
CMAKE_VERSION=3.6.3
# Location to install CMake to.
INSTALL_PREFIX="/opt/cmake"

# We split the version on periods and then extract the first and second
# elements.
CMAKE_MAJMIN=${CMAKE_VERSION[(ws:.:)1]}.${CMAKE_VERSION[(ws:.:)2]}

DOWNLOAD_ROOT="/tmp/cmake"
DOWNLOAD_TARGET="${DOWNLOAD_ROOT}/cmake-${CMAKE_VERSION}.sh"
DOWNLOAD_URL="https://cmake.org/files/v${CMAKE_MAJMIN}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh"

mkdir -p "${DOWNLOAD_ROOT}"
mkdir -p "${INSTALL_PREFIX}"

wget -O "${DOWNLOAD_TARGET}" "${DOWNLOAD_URL}"
sh "${DOWNLOAD_TARGET}" "--prefix=${INSTALL_PREFIX}" --exclude-subdir --skip-license

# Sanity check that cmake exists where we think it should.
test -e "${INSTALL_PREFIX}/bin/cmake"

rm -f -r "${DOWNLOAD_ROOT}"
