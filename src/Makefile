#
# libsensact - the sensor/actuator library
#

CC=gcc
CFLAGS=-Wall -lusb -fPIC -DVERSION=$(VERSION) `pkg-config libusb-1.0 --cflags` -Wno-unused-variable -DSENSACT_CLIENT -I./include 
LDFLAGS=-lsensact `pkg-config libusb-1.0 --libs` -L.
VERSION=0.3

objects = list.o sensact.o session.o usb.o packet.o sensact-emulator.o sensact-emulator/emulator_sensors/sensact_emulator_engine.o sensact-emulator/emulator_sensors/sensact_emulator_senshub.o  sensact-emulator/emulator_tests/sensact-emulator-test.o
     
all: libsensact.so
     
debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

libsensact.so: $(objects)
	$(CC) -Wall -shared -Wl,-soname,libsensact.so -o libsensact.so $(objects) -Wl,-init,init

emulator: sensact-emulator/emulator_tests/sensact-emulator-test.o
	$(CC) -Wall sensact-emulator/emulator_tests/sensact-emulator-test.o -o emulator $(LDFLAGS)

test: libsensact.so test.o
	$(CC) -Wall test.o -o test $(LDFLAGS)

clean:
	rm -f *.o libsensact.so test emulator

.PHONY: all clean debug
