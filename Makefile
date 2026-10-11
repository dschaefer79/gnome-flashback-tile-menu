# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 dschaefer79
# Written entirely with Claude (Anthropic's AI assistant).
# Build the gnome-panel module and install everything:
#   make && make install DESTDIR=... [PREFIX=/usr] [SYSCONFDIR=/etc]
NAME = gnome-flashback-tile-menu
VERSION = 1.0
PREFIX ?= /usr
SYSCONFDIR ?= /etc
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
PKG_CONFIG ?= pkg-config
MODULESDIR ?= $(shell $(PKG_CONFIG) --variable=modulesdir libgnome-panel)
MODULE = org.dschaefer79.tile-menu.so

CFLAGS ?= -O2 -g
# Kept apart from CFLAGS and LDFLAGS, which packaging passes on the command line.
MODULE_CFLAGS = -Wall -fPIC $(shell $(PKG_CONFIG) --cflags libgnome-panel) \
	-DVERSION='"$(VERSION)"' -DDATADIR='"$(DATADIR)"'
MODULE_LDFLAGS = -shared -Wl,--no-undefined -Wl,-z,defs
LIBS = $(shell $(PKG_CONFIG) --libs libgnome-panel)

SCRIPTS = flashback-tile flashback-tile-settings flashback-tile-menu-setup \
	resolution/flashback-resolution-set resolution/flashback-resolution-settings

all: applet/$(MODULE)

applet/$(MODULE): applet/tile-menu-applet.c
	$(CC) $(CPPFLAGS) $(MODULE_CFLAGS) $(CFLAGS) -o $@ $< $(MODULE_LDFLAGS) $(LDFLAGS) $(LIBS)

install: all
	install -Dm 0755 applet/$(MODULE) $(DESTDIR)$(MODULESDIR)/$(MODULE)
	install -dm 0755 $(DESTDIR)$(BINDIR)
	install -pm 0755 $(SCRIPTS) $(DESTDIR)$(BINDIR)/
	install -Dpm 0644 -t $(DESTDIR)$(DATADIR)/$(NAME) menus/tiling.menu menus/resolution.menu
	install -Dpm 0644 -t $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/actions icons/*.svg
	install -Dpm 0644 -t $(DESTDIR)$(DATADIR)/applications \
		data/flashback-tile-menu.desktop data/flashback-tile-settings.desktop \
		data/flashback-resolution-menu.desktop data/flashback-resolution-settings.desktop
	install -Dpm 0644 -t $(DESTDIR)$(SYSCONFDIR)/xdg/autostart \
		data/flashback-tile-menu-setup.desktop data/flashback-tile-snap.desktop
	install -Dpm 0644 resolution/resolutions $(DESTDIR)$(SYSCONFDIR)/$(NAME)/resolutions

clean:
	rm -f applet/$(MODULE)

.PHONY: all install clean
