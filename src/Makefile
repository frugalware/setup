CFLAGS ?= -Wall -O2 -pipe
LDFLAGS = -ldl
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

all: setup
	$(MAKE) -C plugins

setup: setup.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f setup *.o
	$(MAKE) -C plugins clean
