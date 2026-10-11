#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 dschaefer79
# Written entirely with Claude (Anthropic's AI assistant).
# Build the packages for every supported system, each in a podman container
# of that system, from this source tree:
#   packaging/build-all.sh [OUTDIR]     (default: dist)
# Fedora 44 and 45: .rpm and .src.rpm. Debian 13, Ubuntu 26.04 and 26.10:
# .deb. The gnome-panel module is built against each system's gnome-panel,
# so every system gets its own package.
set -eu
here=$(cd "$(dirname "$0")/.." && pwd)
out=$(mkdir -p "${1:-dist}" && cd "${1:-dist}" && pwd)
name=gnome-flashback-tile-menu
version=$(sed -n 's/^Version: *//p' "$here/$name.spec")
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# The source tarball: the files git knows of, under name-version/.
tarball=$work/$name-$version.tar.gz
(cd "$here" && git -c safe.directory="$here" ls-files --cached --others --exclude-standard -z |
    tar czf "$tarball" --null -T - --transform "s|^|$name-$version/|")
cp "$tarball" "$out/"
cp "$here/packaging/build-deb.sh" "$work/"

rpm() {  # fedora version
    podman run --rm -v "$work:/work:Z" -v "$out:/out:Z" \
        "registry.fedoraproject.org/fedora:$1" sh -euc '
        dnf -y -q install rpm-build gcc make "pkgconfig(libgnome-panel)" \
            desktop-file-utils python3 >/dev/null
        rpmbuild -ta --define "_topdir /tmp/rpmbuild" /work/*.tar.gz
        cp /tmp/rpmbuild/RPMS/*/*.rpm /tmp/rpmbuild/SRPMS/*.rpm /out/'
}

deb() {  # image suffix
    podman run --rm -v "$work:/work:Z" -v "$out:/out:Z" "docker.io/library/$1" sh -euc '
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq --no-install-recommends build-essential pkg-config \
            libgnome-panel-dev dpkg-dev >/dev/null
        sh /work/build-deb.sh /work/*.tar.gz "$0" /out "$1"' "$version" "$2"
}

rpm 44
rpm 45
deb debian:trixie deb13
deb ubuntu:26.04 ubuntu26.04
deb ubuntu:26.10 ubuntu26.10
ls -l "$out"
