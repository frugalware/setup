# Makefile for Frugalware Linux Setup
#
# Copyright (C) 2005, 2006, 2007, 2008 by Miklos Vajna <vmiklos@frugalware.org>
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
ifeq ($(DEBUG),false)
CFLAGS ?= -Wall -Werror -O2 -pipe
else
CFLAGS ?= -g -Wall -Werror
endif
CFLAGS += -I.. -DARCH="\"$(shell arch)\"" -DARCH_$(shell arch|tr [a-z] [A-Z])
LDFLAGS += -lparted -ldl -lpacman
CFLAGS += $(shell pkg-config --cflags blkid)
LDFLAGS += $(shell pkg-config --libs blkid)
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

PLUGINS=asklang dolangsh greet layout loadsource configsource partdisk \
	formatdisk select install postconf restart hotplug

all: $(addsuffix .so,$(PLUGINS))

clean:
	rm -f *.so *.o

test: all
	cd ..; PATH=$$PATH:/sbin:/usr/sbin sudo ./setup

%.so: %.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(basename $@).so -o \
		$(basename $@).so $(basename $@).o $(LDFLAGS)
