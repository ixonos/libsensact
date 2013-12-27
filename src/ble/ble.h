#ifndef BLE_H
#define BLE_H

#define HCI_DEVICE_ID 0

#define ATT_CID 4

#define BDADDR_LE_PUBLIC            0x01
#define BDADDR_LE_RANDOM            0x02

struct ble_t
{
    int l2capSock;
};

int ble_get_float(char *feature, double *value, int timeout);
int ble_connect(char *addr);
int ble_disconnect(/*int handle*/);
int ble_reconnect(int handle);
int ble_write(/*int handle,*/ unsigned char *data, int length, int timeout);
int ble_read(/*int handle,*/ unsigned char *data, int length, int timeout);

#endif
