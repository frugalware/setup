CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I..
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif
CFLAGS += $(FINAL)
LDFLAGS=-ldialog -lcurses -lm
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

PLUGINS=asklang dolangsh greet layout

all: $(PLUGINS)

asklang: asklang.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,asklang.so -o 01asklang.so \
		asklang.o ../util.o $(LDFLAGS)

dolangsh: dolangsh.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,dolangsh.so -o 05dolangsh.so \
		dolangsh.o ../util.o $(LDFLAGS)

greet: greet.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,greet.so -o 10greet.so \
		greet.o ../util.o $(LDFLAGS)

layout: layout.o ../util.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,layout.so -o 15layout.so \
		layout.o ../util.o $(LDFLAGS)

clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so

test: all
	cd ..; sudo ./setup
