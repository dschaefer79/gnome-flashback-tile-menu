# gnome-flashback-tile-menu

Two applets for the GNOME Flashback panel (gnome-panel with Metacity):
**Window Tiling** arranges the windows of the current workspace, and
**Resolution** switches the screen resolution with xrandr.

The GNOME Flashback member of a family of ports:
[xfce-tile-menu](https://github.com/dschaefer79/xfce-tile-menu),
[gnome-tile-menu](https://github.com/dschaefer79/gnome-tile-menu) (GNOME Shell),
[cinnamon-tile-menu](https://github.com/dschaefer79/cinnamon-tile-menu) and
[plasma-tile-menu](https://github.com/dschaefer79/plasma-tile-menu).

This project was written entirely with
[Claude Code](https://claude.com/claude-code), Anthropic's AI coding
assistant: all code, packaging and documentation.

## Tiling

- **Layouts:** tile automatically, or into 4, 3 or 2 columns, 3 or 2 rows, or a
  2 × 2, 3 × 2 or 4 × 2 grid. "Automatically" picks the number of columns and
  rows from the screen size (3 columns from 5120 pixels wide, otherwise 2, by
  default).
- **All windows are tiled:** the most recently used windows fill the layout,
  and the others go into the cell under their middle, behind them.
- **Restore windows** puts every window back where it was.
- **Tile new windows:** each window that opens is tiled with the others, in
  the layout last used on its screen.
- **Snapping:** a window dragged with the mouse and dropped fills the column,
  row or grid cell under the pointer. Optionally only with Shift held, or with
  Shift swapping columns and rows.
- **Resize neighbouring windows together:** dragging the edge of a window also
  moves the edge of the window beside it.
- **Gap** between windows, in pixels or millimetres.
- **Metacity edge tiling** (a window dragged to the left or right edge fills
  that half) and **window effects** (Metacity's compositor: shadows,
  transparency, fading) can be turned on or off in the settings.
- **Keyboard shortcuts** for every action, as GNOME custom shortcuts, shown in
  the menu. Default keys: Super+Alt+A (auto), Super+Alt+2/3/4 (columns),
  Super+Ctrl+Alt+2/3 (rows), Super+Ctrl+2/3/4 (grids), Super+Alt+0 (restore),
  only where the desktop does not already use the key.
- **Disable tiling** turns all tiling and snapping off; Restore still works.

## Resolution

- Switches to the resolutions in your list (1920 × 1080, 1900 × 1000 and
  5120 × 1440 by default). Sizes the screen does not offer are added to it with
  xrandr, when picked or right away, which also makes it work in TigerVNC
  sessions.
- Edit the list in its settings: add, remove, edit and reorder.

## Settings and menus

- The applications menu has **Window Tiling** and **Resolution Menu
  Settings** among the settings, and both menus have **Settings…**. Tiling
  settings take effect with **Apply**.
- **Show … menu on the panel** in each settings window puts that applet on
  the panel or takes it off.
- **Save settings to file… / Load settings from file…** keep a copy of the
  settings of both menus and the shortcuts, or move them to another computer.
  Files saved by xfce-tile-menu load too.
- **Quit** at the bottom of each menu takes the applet off the panel, as does
  "Remove From Panel" in its right-click menu. **Window Tiling Menu** and
  **Resolution Menu** in the applications menu put them back.
  They can also be added like any other applet: right-click the panel, *Add
  to Panel…*, then *Window Tiling* or *Resolution*.

## Installing

From the [latest GitHub release](https://github.com/dschaefer79/gnome-flashback-tile-menu/releases/latest).
The applets are a gnome-panel module built against each system's
gnome-panel, so every system has its own package:

| System | gnome-panel | Package |
|---|---|---|
| Fedora 45 | 3.58 | `gnome-flashback-tile-menu-1.0-1.fc45.x86_64.rpm` |
| Fedora 44 | 3.58 | `gnome-flashback-tile-menu-1.0-1.fc44.x86_64.rpm` |
| Ubuntu 26.10 | 3.58 | `gnome-flashback-tile-menu_1.0-1+ubuntu26.10_amd64.deb` |
| Ubuntu 26.04 LTS | 3.58 | `gnome-flashback-tile-menu_1.0-1+ubuntu26.04_amd64.deb` |
| Debian 13 (trixie) | 3.56 | `gnome-flashback-tile-menu_1.0-1+deb13_amd64.deb` |

On Fedora (`fc44` instead of `fc45` on Fedora 44):

```
sudo dnf install https://github.com/dschaefer79/gnome-flashback-tile-menu/releases/download/v1.0/gnome-flashback-tile-menu-1.0-1.fc45.x86_64.rpm
```

On Ubuntu (`26.04` instead of `26.10` on Ubuntu 26.04):

```
wget https://github.com/dschaefer79/gnome-flashback-tile-menu/releases/download/v1.0/gnome-flashback-tile-menu_1.0-1+ubuntu26.10_amd64.deb
sudo apt install ./gnome-flashback-tile-menu_1.0-1+ubuntu26.10_amd64.deb
```

On Debian 13:

```
wget https://github.com/dschaefer79/gnome-flashback-tile-menu/releases/download/v1.0/gnome-flashback-tile-menu_1.0-1+deb13_amd64.deb
sudo apt install ./gnome-flashback-tile-menu_1.0-1+deb13_amd64.deb
```

GNOME Flashback itself is `gnome-flashback` on Fedora,
`gnome-session-flashback` on Ubuntu and Debian; pick *GNOME Flashback
(Metacity)* on the login screen. GNOME Flashback runs on X11 only.

The applets appear on the top panel right after installing (the panel
restarts once to load them) or at the next login, for every user. Run
`flashback-tile-menu-setup` to add them yourself.

## Building

`packaging/build-all.sh` builds all five packages, each in a podman
container of its system, into `dist/`. For one system only:

- **Fedora:** `rpmbuild -ta gnome-flashback-tile-menu-1.0.tar.gz` (needs
  `gnome-panel-devel`, `gcc`, `make`, `desktop-file-utils`).
- **Debian / Ubuntu:** `packaging/build-deb.sh TARBALL 1.0 OUTDIR SUFFIX`
  (needs `libgnome-panel-dev`, `build-essential`, `pkg-config`, `dpkg-dev`).
- **Any system:** `make && sudo make install`.

The tarball holds this tree under `gnome-flashback-tile-menu-1.0/`, for
example `git archive --prefix=gnome-flashback-tile-menu-1.0/ -o
gnome-flashback-tile-menu-1.0.tar.gz HEAD`.

## Command line

`flashback-tile auto|4|3|2|rows-3|rows-2|grid-2x2|grid-3x2|grid-4x2|restore`
tiles; `flashback-tile toggle snap|tile-new|disabled|vertical|horizontal`,
`flashback-tile menu`, `flashback-tile export FILE` and `flashback-tile import
FILE`. `flashback-resolution-set WIDTHxHEIGHT` switches the resolution.
`flashback-tile-menu-setup [tiling|resolution]` puts the applets on the panel
(`--remove` takes one off, `--status` says whether it is there).

## How it works

- `applet/tile-menu-applet.c` is the gnome-panel module
  (`org.dschaefer79.tile-menu`) with the two applets. Each shows an icon and,
  when clicked, a menu read from `~/.config/gnome-flashback-tile-menu/tiling.menu`
  or `resolution.menu`.
- `flashback-tile` (Python, GTK 3, libwnck) does the tiling, snapping and
  restoring, and writes the menus from the settings and shortcuts.
- `flashback-tile-settings` and `flashback-resolution-settings` are the
  settings windows; `flashback-tile-menu-setup` adds the applets to the
  panel's layout in dconf.

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
Copyright (C) 2026 dschaefer79.
