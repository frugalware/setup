CFLAGS ?= -Wall -march=i686 -O2 -pipe
CFLAGS += -I..

PLUGINS=foo bar

all: $(PLUGINS)

foo: foo.o
	$(CC) -shared -Wl,-soname,foo.so -o foo.so foo.o
	
bar: bar.o
	$(CC) -shared -Wl,-soname,bar.so -o bar.so bar.o

clean:
	rm -f $(PLUGINS) *.o
