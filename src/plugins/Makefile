CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I..
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif
LDFLAGS=-ldialog -lcurses -lm
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

PLUGINS=asklang

all: $(PLUGINS)

asklang: asklang.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,asklang.so -o 01asklang.so \
		asklang.o ../util.o $(LDFLAGS)
	
clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so

test: all
	cd ..; ./setup
