CC=$(PREFIX)gcc
CFLAGS=-Wall -Wno-unused-variable -Wno-unused-but-set-variable

SA_INCLUDE=-I../include
SA_PLUGIN_C_FILE=../plugin.c

all: plugin.o sensact-usb.so

plugin.o: $(SA_PLUGIN_C_FILE)
	$(CC) $(CFLAGS) $(SA_INCLUDE) -fPIC -c $(SA_PLUGIN_C_FILE)

sensact-usb.so: usb.c plugin.o
	$(CC) $(CFLAGS) $(SA_INCLUDE) -fPIC -c usb.c `pkg-config libusb-1.0 --cflags` 
	$(CC) -shared -Wl,-soname,sensact-usb.so -o sensact-usb.so usb.o plugin.o `pkg-config libusb-1.0 --libs`

clean:
	rm -f *.o *.so

.PHONY: all clean
