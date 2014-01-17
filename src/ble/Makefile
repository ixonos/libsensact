#
# libsensact - the sensor/actuator library
#

CC=gcc
CFLAGS=-Wall -lbluetooth -fPIC -Wno-unused-variable
LDFLAGS=-lbluetooth -lm

objects = ble.o TI_sensortag.o gatt_att.o
     
all: ble_demo
     
debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

ble_demo: $(objects) ble_demo.o
	$(CC) -Wall $(objects) ble_demo.o -o ble_demo $(LDFLAGS)

ble_test: $(objects) ble_test.o
	$(CC) -Wall $(objects) ble_test.o -o ble_test $(LDFLAGS)
	
clean:
	rm -f *.o ble_demo ble_test

.PHONY: all clean debug
