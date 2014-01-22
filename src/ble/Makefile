#
# libsensact - the sensor/actuator library
#

CC=gcc
CFLAGS=-Wall -fPIC -Wno-unused-variable
LDFLAGS=-lm -lbluetooth -pthread
AR=ar

objects = ble.o TI_sensortag.o gatt_att.o
     
all: ble_demo
     
debug: CFLAGS += -DDEBUG -g
debug: all

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

ble_update.a: $(objects) ble_update.o
	$(AR) rcs ble_update.a $(objects) ble_update.o

ble_demo: $(objects) ble_demo.o ble_update.a
	$(CC) -Wall $(objects) ble_demo.o ble_update.a -o ble_demo $(LDFLAGS)

ble_test: $(objects) ble_test.o
	$(CC) -Wall $(objects) ble_test.o -o ble_test $(LDFLAGS)

ble_temp: $(objects) ble_temp.o
	$(CC) -Wall $(objects) ble_temp.o -o ble_temp $(LDFLAGS)
	
clean:
	rm -f *.o ble_demo ble_test ble_temp ble_update.a

.PHONY: all clean debug
