#
# libsensact - the sensor/actuator library
#

CC=$(PREFIX)gcc
CFLAGS=-Wall -fPIC -DVERSION=$(VERSION) -Wno-unused-variable -DSENSACT_CLIENT -I./include
LDFLAGS=-L. -lsensact
VERSION=0.3

objects = sensact.o session.o packet.o list.o plugin-manager.o

all: libsensact.so plugins

debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

libsensact.so: $(objects)
	$(CC) -Wall -shared -Wl,-soname,libsensact.so -o libsensact.so $(objects) -Wl,-init,init -ldl

test: libsensact.so test.o
	$(CC) -Wall test.o -o test $(LDFLAGS)

plugins:
	make -C plugins

clean:
	rm -f *.o libsensact.so test
	make -C plugins clean

.PHONY: all clean debug plugins
