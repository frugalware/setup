CFLAGS ?= -Wall -O2 -pipe
LDFLAGS = -ldl
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)
CFLAGS += -DDIALOG
LDFLAGS += -ldialog -lcurses -lm -rdynamic
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

all: setup
	$(MAKE) -C plugins

setup: setup.o util.o
	$(CC) $(CFLAGS) -o $@ setup.o util.o $(LDFLAGS)

clean:
	rm -f setup *.o
	$(MAKE) -C plugins clean
