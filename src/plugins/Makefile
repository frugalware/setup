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
CFLAGS += -I.. $(FINAL) -DARCH="\"$(shell arch)\""
LDFLAGS += -lparted -ldl
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

ifeq ($(UI),dialog)
CFLAGS += -DDIALOG
LDFLAGS += -ldialog -lcurses -lm
else
CFLAGS += -DGTK
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
endif

CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

PLUGINS=asklang dolangsh greet layout hotplug loadsource configsource partdisk \
	formatdisk restart

all: $(addsuffix .so,$(PLUGINS))

clean:
	rm -f *.so *.o

test: all
	cd ..; PATH=$$PATH:/sbin:/usr/sbin sudo ./setup

%.so:
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so -o \
		$(basename $@).so $(basename $@).c $(LDFLAGS)
