UI ?= dialog
CFLAGS ?= -Wall -O2 -pipe
CFLAGS += $(FINAL) -DARCH="\"$(shell arch)\""
LDFLAGS += -ldl -rdynamic
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

ifeq ($(UI),dialog)
CFLAGS += -DDIALOG
LDFLAGS += -ldialog -lcurses -lm
else
CFLAGS += -DGTK
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS += $(shell pkg-config --libs gtk+-2.0)
endif

ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

all: setup
	$(MAKE) -C plugins FINAL="$(FINAL)" UI="$(UI)"

setup: setup.o util.o
	$(CC) $(CFLAGS) -o $@ setup.o util.o $(LDFLAGS)

final:
	$(MAKE) FINAL="-DFINAL" UI="$(UI)"

clean:
	rm -f setup *.o
	$(MAKE) -C plugins clean
