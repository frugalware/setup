CFLAGS ?= -Wall -O2 -pipe
CFLAGS += -I..
ifeq ($(shell arch),x86_64)
CFLAGS += -fPIC
endif

PLUGINS=asklang bar

all: $(PLUGINS)

asklang: asklang.o
	$(CC) -shared -Wl,-soname,asklang.so -o asklang.so asklang.o
	
bar: bar.o
	$(CC) -shared -Wl,-soname,bar.so -o bar.so bar.o

clean:
	rm -f $(PLUGINS) *.o
	rm -f *.so
