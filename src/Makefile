#
# libsensact - the sensor/actuator library
#

CC=gcc
CFLAGS=-Wall -fPIC -DVERSION=$(VERSION) `pkg-config libusb-1.0 --cflags` -Wno-unused-variable -DSENSACT_CLIENT -I./include
LDFLAGS=-L. -lsensact -lbluetooth -lpthread -lm `pkg-config libusb-1.0 --libs`
VERSION=0.3

objects = sensact.o session.o usb.o packet.o list.o ble_sensortag.o
     
all: libsensact.so
     
debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

libsensact.so: $(objects)
	make -C ble ble_update.a
	$(CC) -Wall -shared -Wl,-soname,libsensact.so -o libsensact.so $(objects) ble/ble_update.a -Wl,-init,init -lm

test: libsensact.so test.o
	$(CC) -Wall test.o -o test $(LDFLAGS)

ble_test: libsensact.so ble_test.o
	$(CC) -Wall ble_test.o -o ble_test $(LDFLAGS)

clean:
	make -C ble clean
	rm -f *.o libsensact.so test ble_test

.PHONY: all clean debug
