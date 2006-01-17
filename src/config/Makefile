CFLAGS ?= -Wall -march=i686 -O2 -pipe
LDFLAGS=-ldialog -lcurses -lm

TOOLS = netconfig

all: $(TOOLS)

netconfig: netconfig.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TOOLS) *.o
