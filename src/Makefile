#
# libsensact - the sensor/actuator library
#


CC=$(PREFIX)gcc
CFLAGS=-Wall -std=gnu99 -DVERSION=$(VERSION) `pkg-config libusb-1.0 --cflags` -Wno-unused-variable -DSENSACT_CLIENT -I./include
LDFLAGS=-L. -lsensact -lbluetooth -lpthread -lm `pkg-config libusb-1.0 --cflags` 
VERSION=0.3

objects = sensact.o session.o packet.o list.o plugin-manager.o ble_sensortag.o
all: libsensact.so plugins

debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libsensact.so: $(objects)
	make -C ble ble_update.a
	$(CC) $(CFLAGS) -shared -Wl,-soname,libsensact.so -o libsensact.so $(objects) ble/ble_update.a -Wl,-init,init -lm -ldl

test: libsensact.so test.o
	$(CC) -Wall test.o -o test $(LDFLAGS)

plugins:
	make -C plugins 

clean:
	rm -f *.o libsensact.so test ble_test
	make -C plugins clean
	make -C ble clean

ble_test: libsensact.so ble_test.o
	$(CC) -Wall ble_test.o -o ble_test $(LDFLAGS)

.PHONY: all clean debug plugins
