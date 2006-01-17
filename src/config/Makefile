CFLAGS ?= -Wall -march=i686 -O2 -pipe
LDFLAGS=-ldialog -lcurses -lm
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

TOOLS = netconfig

all: $(TOOLS)

netconfig: netconfig.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TOOLS) *.o
