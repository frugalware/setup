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
	restart

all: $(PLUGINS)
	@echo Build completed.

clean:
	rm -f *.so *.o

test: all
	cd ..; PATH=$$PATH:/sbin:/usr/sbin sudo ./setup

%:
	$(CC) $(CFLAGS) -shared -Wl,-soname,$@.so -o $@.so $@.c $(LDFLAGS)
