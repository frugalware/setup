CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I..
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

PLUGINS=asklang bar

all: $(PLUGINS)

asklang: asklang.o
	$(CC) -shared -Wl,-soname,asklang.so -o 01asklang.so $<
	
bar: bar.o
	$(CC) -shared -Wl,-soname,bar.so -o 50bar.so $<

clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so

test: all
	cd ..; ./setup
