CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I.. $(FINAL) -DARCH="\"$(shell arch)\""
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

LDFLAGS=-ldialog -lcurses -lm
LDFLAGS += -lparted -ldl

CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

PLUGINS=asklang dolangsh greet layout hotplug loadsource configsource partdisk \
	restart

all: $(PLUGINS)
	@echo Build completed.

clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so

test: all
	cd ..; PATH=$$PATH:/sbin:/usr/sbin sudo ./setup

%:
	$(CC) $(CFLAGS) -shared -Wl,-soname,$@.so -o $@.so $@.c $(LDFLAGS)
