# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 dschaefer79
# Written entirely with Claude (Anthropic's AI assistant).

Name:           gnome-flashback-tile-menu
Version:        1.0
Release:        1%{?dist}
Summary:        GNOME Flashback panel menus for window tiling and screen resolution
License:        GPL-3.0-or-later
URL:            https://github.com/dschaefer79/gnome-flashback-tile-menu
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pkgconfig(libgnome-panel)
BuildRequires:  desktop-file-utils
BuildRequires:  python3
Requires:       gnome-panel%{?_isa}
Requires:       metacity
Requires:       gnome-settings-daemon
Requires:       python3-gobject
Requires:       gtk3
Requires:       libwnck3
Requires:       libnotify
Requires:       procps-ng
# The resolution menu
Requires:       xrandr
Recommends:     cvt
Recommends:     gnome-flashback
Requires(posttrans): procps-ng
Requires(posttrans): /usr/sbin/runuser

%description
Two applets for the GNOME Flashback panel (gnome-panel with Metacity), the
counterparts of plasma-tile-menu's widgets.

Window Tiling tiles the windows of the current workspace into side-by-side
columns, rows stacked top to bottom, or a grid of both: automatically (3
columns on screens at least 5120 pixels wide, such as 5120x1440, and 2
otherwise; rows follow the same rule by screen height), or into 4, 3 or 2
columns, 3 or 2 rows, or a 2x2, 3x2 or 4x2 grid directly, and puts them
back with Restore. New windows can be tiled automatically as they open,
into the layout last used on their screen. The settings window turns
columns and rows on or off, sets the auto rules and the gap between windows
(in pixels or millimetres), and can turn on snapping: a window dragged with
the mouse and dropped fills the column, row or grid cell under the pointer.
Snapping can be limited to drops with Shift held, and Shift can swap
columns and rows. Every action can have a keyboard shortcut (GNOME custom
shortcuts), shown in the menu.

Resolution switches the display resolution with xrandr from a list
(1920x1080, 1900x1000 and 5120x1440 by default). Its settings window adds,
removes, edits and reorders resolutions, and adds new ones to the screen
right away or at the press of a button. Missing modes are added
automatically, which also makes it work in TigerVNC sessions.

Both applets are added to the top panel for each user at their next login,
or right away by running flashback-tile-menu-setup.

This project was written entirely with Claude, Anthropic's AI assistant.

%prep
%autosetup

%build
%make_build CFLAGS="%{optflags}" LDFLAGS="%{build_ldflags}"

%install
%make_install PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/*.desktop
desktop-file-validate %{buildroot}%{_sysconfdir}/xdg/autostart/*.desktop
for f in %{buildroot}%{_bindir}/flashback-tile %{buildroot}%{_bindir}/flashback-tile-settings \
         %{buildroot}%{_bindir}/flashback-tile-menu-setup %{buildroot}%{_bindir}/flashback-resolution-settings; do
    python3 -c 'import ast, sys; ast.parse(open(sys.argv[1]).read())' "$f"
done

%posttrans
# For users logged in to GNOME Flashback, put the applets on the panel
# (restarting a panel that does not know them yet), rebuild the menus and
# restart the snap daemon, so the new version is used without logging out.
for pid in $(pgrep -x gnome-panel 2>/dev/null); do
    user=$(id -nu "$(ps -o uid= -p "$pid" | tr -d ' ')" 2>/dev/null) || continue
    env=$(tr '\0' '\n' <"/proc/$pid/environ" 2>/dev/null |
        grep -E '^(DISPLAY|DBUS_SESSION_BUS_ADDRESS|XAUTHORITY|XDG_RUNTIME_DIR|HOME|PATH)=') || continue
    pkill -u "$user" -f 'flashback-tile snap-daemon' 2>/dev/null
    runuser -u "$user" -- env $env sh -c \
        'flashback-tile-menu-setup all --auto >/dev/null 2>&1; setsid flashback-tile snap-daemon >/dev/null 2>&1 &' || :
done
:

%files
%license LICENSE
%doc README.md
%{_libdir}/gnome-panel/modules/org.dschaefer79.tile-menu.so
%{_bindir}/flashback-tile
%{_bindir}/flashback-tile-settings
%{_bindir}/flashback-tile-menu-setup
%{_bindir}/flashback-resolution-set
%{_bindir}/flashback-resolution-settings
%{_datadir}/%{name}/
%{_datadir}/applications/flashback-tile-menu.desktop
%{_datadir}/applications/flashback-tile-settings.desktop
%{_datadir}/applications/flashback-resolution-menu.desktop
%{_datadir}/applications/flashback-resolution-settings.desktop
%{_datadir}/icons/hicolor/scalable/actions/flashback-tile-columns-*-symbolic.svg
%{_datadir}/icons/hicolor/scalable/actions/flashback-tile-rows-*-symbolic.svg
%{_datadir}/icons/hicolor/scalable/actions/flashback-tile-grid-*-symbolic.svg
%config(noreplace) %{_sysconfdir}/xdg/autostart/flashback-tile-menu-setup.desktop
%config(noreplace) %{_sysconfdir}/xdg/autostart/flashback-tile-snap.desktop
%dir %{_sysconfdir}/%{name}
%config(noreplace) %{_sysconfdir}/%{name}/resolutions

%changelog
* Sun Oct 11 2026 dschaefer79 <dschaefer79@gmail.com> - 1.0-1
- Initial package: GNOME Flashback port of xfce-tile-menu 2.17, with a
  gnome-panel module for the Window Tiling and Resolution applets
