#
# libsensact - the sensor/actuator library
#

CC=gcc
CFLAGS=-Wall -lusb -fPIC -DVERSION=$(VERSION) `pkg-config libusb-1.0 --cflags` -Wno-unused-variable -DSENSACT_CLIENT
LDFLAGS=-lsensact `pkg-config libusb-1.0 --libs` -L.
VERSION=0.2

objects = sensact.o session.o usb.o packet.o
     
all: libsensact.so
     
debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

libsensact.so: $(objects)
	$(CC) -Wall -shared -Wl,-soname,libsensact.so -o libsensact.so $(objects) -Wl,-init,init

test: libsensact.so test.o
	$(CC) -Wall test.o -o test $(LDFLAGS)

clean:
	rm -f *.o libsensact.so test

.PHONY: all clean debug
