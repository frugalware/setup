CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I..
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

PLUGINS=foo bar

all: $(PLUGINS)

foo: foo.o
	$(CC) -shared -Wl,-soname,foo.so -o foo.so foo.o
	
bar: bar.o
	$(CC) -shared -Wl,-soname,bar.so -o bar.so bar.o

clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so
