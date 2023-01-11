#!/bin/sh

set -ex

VERSION="$(git describe --tags)"

PREFIX="mvdsv-${VERSION}/"

DIST_MVDSV="mvdsv-${VERSION}.tar"
DIST_QWPROT="qwprot-${VERSION}.tar"

git submodule init
git submodule update

git archive --format="tar" --prefix="${PREFIX}" HEAD > "${DIST_MVDSV}"
git --git-dir="qwprot/.git" archive --format="tar" --prefix="${PREFIX}qwprot/" HEAD > "${DIST_QWPROT}"

tar -Af "${DIST_MVDSV}" "${DIST_QWPROT}"

rm -f "${DIST_QWPROT}" "${DIST_MVDSV}.gz"

gzip -9 "${DIST_MVDSV}"
