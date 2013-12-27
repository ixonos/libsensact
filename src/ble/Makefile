#
# libsensact - the sensor/actuator library
#

CC=gcc
CFLAGS=-Wall -lbluetooth -fPIC -Wno-unused-variable
LDFLAGS=-lbluetooth -lm

objects = ble_test.o ble.o TI_sensortag.o gatt_att.o
     
all: ble_test
     
debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

ble_test: $(objects)
	$(CC) -Wall $(objects) -o ble_test $(LDFLAGS)

clean:
	rm -f *.o ble_test

.PHONY: all clean debug
