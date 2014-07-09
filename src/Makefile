#
# libsensact - the sensor/actuator library
#

CC=$(PREFIX)gcc
CFLAGS=-Wall -std=gnu99 -fPIC -DVERSION=$(VERSION)  -Wno-unused-variable -DSENSACT_CLIENT -I./include
LDFLAGS=-L. -lsensact
VERSION=0.3

emuobj = sensact-emulator.o sensact-emulator/emulator_sensors/sensact_emulator_ble.o sensact-emulator/emulator_sensors/sensact_emulator_engine.o sensact-emulator/emulator_sensors/sensact_emulator_senshub.o  sensact-emulator/emulator_tests/sensact-emulator-test.o
objects = sensact.o session.o packet.o list.o plugin-manager.o $(emuobj)
all: libsensact.so plugins

debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libsensact.so: $(objects)
	$(CC) $(CFLAGS) -shared -Wl,-soname,libsensact.so -o libsensact.so $(objects) -Wl,-init,init -ldl

emulator: sensact-emulator/emulator_tests/sensact-emulator-test.o
	$(CC) -Wall sensact-emulator/emulator_tests/sensact-emulator-test.o -o emulator $(LDFLAGS)

test: libsensact.so test.o
	$(CC) -Wall test.o -o test $(LDFLAGS)

plugins:
	make -C plugins 

clean:
	rm -f *.o libsensact.so test emulator
	rm -f sensact-emulator/*/*.o
	make -C plugins clean

.PHONY: all clean debug plugins
