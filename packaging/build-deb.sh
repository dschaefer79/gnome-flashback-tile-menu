#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 dschaefer79
# Written entirely with Claude (Anthropic's AI assistant).
# Build gnome-flashback-tile-menu as a .deb on the system it is for (the
# gnome-panel module is built against that system's gnome-panel).
#   build-deb.sh SOURCE_TARBALL VERSION OUTDIR [SUFFIX]
# The tarball is a git archive with the top directory
# gnome-flashback-tile-menu-VERSION. SUFFIX goes after the Debian revision,
# such as deb13 or ubuntu26.04. Needs build-essential, pkg-config,
# libgnome-panel-dev and dpkg-dev.
set -eu
tarball=$1 version=$2 out=$3 suffix=${4:-}
name=gnome-flashback-tile-menu
revision=1${suffix:++$suffix}
work=$(mktemp -d)
tar xzf "$tarball" -C "$work"
src=$work/$name-$version
root=$work/root

eval "$(dpkg-buildflags --export=sh)"
make -C "$src" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS"
make -C "$src" install DESTDIR="$root" PREFIX=/usr SYSCONFDIR=/etc
module=$(find "$root" -name 'org.dschaefer79.tile-menu.so')
strip --strip-unneeded --remove-section=.comment --remove-section=.note "$module"
chmod 0644 "$module"

# Documentation, as Debian wants it.
doc=$root/usr/share/doc/$name
install -Dm 0644 "$src/README.md" "$doc/README.md"
cat > "$doc/copyright" <<EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: $name
Upstream-Contact: dschaefer79 <7472357+dschaefer79@users.noreply.github.com>
Source: https://github.com/dschaefer79/$name
Comment: Written entirely with Claude Code, Anthropic's AI coding assistant.

Files: *
Copyright: 2026 dschaefer79
License: GPL-3.0-or-later

License: GPL-3.0-or-later
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 .
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 On Debian systems, the complete text of the GNU General Public License
 version 3 can be found in "/usr/share/common-licenses/GPL-3".
EOF
. /etc/os-release
cat > "$work/changelog" <<EOF
$name ($version-$revision) stable; urgency=medium

  * Package for $PRETTY_NAME, built from
    https://github.com/dschaefer79/$name tag v$version.

 -- dschaefer79 <7472357+dschaefer79@users.noreply.github.com>  $(date -R)
EOF
gzip -9n -c "$work/changelog" > "$doc/changelog.Debian.gz"
chmod 0644 "$doc/changelog.Debian.gz"

# The libraries the module links, as package dependencies.
mkdir -p "$work/debian"
printf 'Source: %s\n\nPackage: %s\nArchitecture: any\n' "$name" "$name" > "$work/debian/control"
shlibs=$(cd "$work" && dpkg-shlibdeps -O "${module#"$work"/}" | sed -n 's/^shlibs:Depends=//p')

# Control files.
mkdir -p "$root/DEBIAN"
size=$(du -sk --exclude=DEBIAN "$root" | cut -f1)
cat > "$root/DEBIAN/control" <<EOF
Package: $name
Version: $version-$revision
Architecture: $(dpkg --print-architecture)
Maintainer: dschaefer79 <7472357+dschaefer79@users.noreply.github.com>
Installed-Size: $size
Depends: $shlibs, gnome-panel, metacity, gnome-settings-daemon, python3, python3-gi, gir1.2-gtk-3.0, gir1.2-wnck-3.0, libnotify-bin, procps, x11-xserver-utils
Recommends: gnome-flashback, xcvt
Section: gnome
Priority: optional
Homepage: https://github.com/dschaefer79/$name
Description: GNOME Flashback panel menus for window tiling and screen resolution
 Two applets for the GNOME Flashback panel. Window Tiling tiles the windows
 of the current workspace into columns, rows or a grid (automatically, 2,
 3 or 4 columns, 2 or 3 rows, a 2 x 2, 3 x 2 or 4 x 2 grid), puts them back
 with Restore, can tile new windows as they open and snap dropped windows
 into place, with keyboard shortcuts for every action. Resolution switches
 the screen resolution with xrandr from a list you edit, adding sizes the
 screen does not offer.
 .
 Both applets are put on the panel at the next login, or right away by
 running flashback-tile-menu-setup.
 .
 Written entirely with Claude Code, Anthropic's AI coding assistant.
EOF
cat > "$root/DEBIAN/conffiles" <<EOF
/etc/xdg/autostart/flashback-tile-menu-setup.desktop
/etc/xdg/autostart/flashback-tile-snap.desktop
/etc/$name/resolutions
EOF
cat > "$root/DEBIAN/postinst" <<'EOF'
#!/bin/sh
# For users logged in to GNOME Flashback, put the applets on the panel
# (restarting a panel that does not know them yet), rebuild the menus and
# restart the snap daemon, so the new version is used without logging out.
set -e
if [ "$1" = configure ]; then
    for pid in $(pgrep -x gnome-panel 2>/dev/null || :); do
        user=$(id -nu "$(ps -o uid= -p "$pid" | tr -d ' ')" 2>/dev/null) || continue
        env=$(tr '\0' '\n' <"/proc/$pid/environ" 2>/dev/null |
            grep -E '^(DISPLAY|DBUS_SESSION_BUS_ADDRESS|XAUTHORITY|XDG_RUNTIME_DIR|HOME|PATH)=') || continue
        pkill -u "$user" -f 'flashback-tile snap-daemon' 2>/dev/null || :
        runuser -u "$user" -- env $env sh -c \
            'flashback-tile-menu-setup all --auto >/dev/null 2>&1; setsid flashback-tile snap-daemon >/dev/null 2>&1 &' || :
    done
fi
exit 0
EOF
chmod 0755 "$root/DEBIAN/postinst"

mkdir -p "$out"
dpkg-deb --root-owner-group --build "$root" \
    "$out/${name}_${version}-${revision}_$(dpkg --print-architecture).deb"
rm -rf "$work"
