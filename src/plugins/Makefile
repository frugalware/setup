CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I..
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif
CFLAGS += $(FINAL) -DARCH="\"$(shell arch)\""
LDFLAGS=-ldialog -lcurses -lm
PEDLDFLAGS=-lparted -ldl
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

PLUGINS=asklang dolangsh greet layout hotplug loadsource configsource partdisk \
	restart

all: $(PLUGINS)

asklang: asklang.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,asklang.so -o asklang.so \
		asklang.o ../util.o $(LDFLAGS)

dolangsh: dolangsh.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,dolangsh.so -o dolangsh.so \
		dolangsh.o ../util.o $(LDFLAGS)

greet: greet.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,greet.so -o greet.so \
		greet.o ../util.o $(LDFLAGS)

layout: layout.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,layout.so -o layout.so \
		layout.o ../util.o $(LDFLAGS)

hotplug: hotplug.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,hotplug.so -o hotplug.so \
		hotplug.o ../util.o $(LDFLAGS)

loadsource: loadsource.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,loadsource.so -o loadsource.so \
		loadsource.o ../util.o $(LDFLAGS)

configsource: configsource.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,configsource.so -o \
		configsource.so configsource.o ../util.o $(LDFLAGS)

partdisk: partdisk.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,partdisk.so -o partdisk.so \
		partdisk.o ../util.o $(LDFLAGS) $(PEDLDFLAGS)

restart: restart.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,restart.so -o restart.so \
		restart.o ../util.o $(LDFLAGS)

clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so

test: all
	cd ..; PATH=$$PATH:/sbin:/usr/sbin sudo ./setup
