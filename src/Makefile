# Makefile for Frugalware Linux Setup
#
# Copyright (C) 2005 by Miklos Vajna <vmiklos@frugalware.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

UI ?= dialog
CFLAGS ?= -Wall -O2 -pipe
CFLAGS += $(FINAL) -DARCH="\"$(shell arch)\""
LDFLAGS += -ldl -rdynamic
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

ifeq ($(UI),dialog)
CFLAGS += -DDIALOG
LDFLAGS += -ldialog -lcurses -lm
else
CFLAGS += -DGTK
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
endif

ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

all: setup
	$(MAKE) -C plugins FINAL="$(FINAL)" UI="$(UI)"

setup: setup.o util.o
	$(CC) $(CFLAGS) -o $@ setup.o util.o $(LDFLAGS)

final:
	$(MAKE) FINAL="-DFINAL" UI="$(UI)"

clean:
	rm -f setup *.o
	$(MAKE) -C plugins clean
