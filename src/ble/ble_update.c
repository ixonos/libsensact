#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ble.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <pthread.h>
#include <signal.h>

// This code is made for demo purposes!

// Disable printf output
//#define printf(x...) 

pthread_t ble_update_thread;

#define TIMEOUT 2 // sec

#define BLE_RW_RETRYCNT    5 // retrials for failed readings/writings to the ble-device
#define BLE_READ_INTERVAL  1 // seconds
#define BLE_SCAN_TIME      20 // seconds

//#define BLE_RECONN_RETRYCNT   10
#define BLE_RECONN_INTERVAL  5 // seconds

float ble_temp = 0; // temperature

enum
{
    BLE_STATE_NOT_CONNECTED = 0,
    BLE_STATE_WF_RECONN = 1,
    BLE_STATE_CONNECTED = 2,
    BLE_STATE_DISCONNECTED = 3,
    BLE_STATE_WF_READ = 4,
    BLE_STATE_READ = 5
};

typedef struct ble_conn_t
{
    char ble_addr[18];
    int state;
    int rw_retry_cnt;
} BLE_CONN_T;

void ble_scan(void)
{
    int dd, err, dev_id;
    uint8_t own_type = 0x00;
    uint8_t scan_type = 0x01;
    uint8_t filter_policy = 0x00;
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);
    uint8_t filter_dup = 1;

    // Open first available hci device
    dev_id = hci_get_route(NULL);
    dd = hci_open_dev(dev_id);
    if (dd < 0)
    {
        perror("Could not open device");
        return;
    }

    err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
                                     own_type, filter_policy, 1000);
    if (err < 0)
    {
        perror("Set scan parameters failed");
        goto error;
    }

    // Start scan
    err = hci_le_set_scan_enable(dd, 0x01, filter_dup, 1000);
    if (err < 0)
    {
        perror("Enable scan failed");
        goto error;
    }

    printf("LE Scan ...\n");
    sleep(BLE_SCAN_TIME);

    // End scan
    err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 1000);
    if (err < 0)
        perror("Disable scan failed");

error:
    hci_close_dev(dd);
}

// This demo-program in simple way to adapt ble-interface to sensact-platform

void update_loop(char *ble_addr, float *temperature)
{

    // this function will handle connection-establishment (connect/reconnect) 
    // and temperature-reading

    // This function will called once, so it should be save to do ble-specific initializations here

    // In endless-loop it starts to follow next procedure: 
    //     BLE_STATE_NOT_CONNECTED -> BLE_STATE_CONNECTED -> BLE_STATE_READ ->
    //     BLE_STATE_WF_READ -> BLE_STATE_READ ->  BLE_STATE_WF_READ -> BLE_STATE_READ -> BLE_STATE_WF_READ -> BLE_STATE_READ ...
    // If problems in reading occur, after BLE_RW_RETRYCNT-retrials: 
    //      BLE_STATE_DISCONNECTED -> BLE_STATE_WF_RECONN -> BLE_STATE_NOT_CONNECTED -> BLE_STATE_CONNECTED -> BLE_STATE_READ  ...


    struct ble_conn_t ble_conn;
    int ret;
    double double_val;

    ble_conn.state = BLE_STATE_NOT_CONNECTED; // ble-connection should not be establishment yet
    strncpy(ble_conn.ble_addr, ble_addr, sizeof(ble_conn.ble_addr));
    ble_conn.rw_retry_cnt = 0;

    // it has simple state-machine for keeping connection-states

    while(1) // endless loop
    {
        switch (ble_conn.state)
        {
            case BLE_STATE_NOT_CONNECTED:
                {
                    printf("--------------------------------------------------------------------------\n");

                    // LE scan before connect (fixes "no route to host")
                    ble_scan();

                    ret = ble_connect(ble_conn.ble_addr);

                    if (ret < 0) // connection establishment failed
                        ble_conn.state = BLE_STATE_WF_RECONN;
                    else
                        ble_conn.state = BLE_STATE_CONNECTED;
                }
                break;

            case BLE_STATE_CONNECTED:
                {
                    printf("ble connected\n");
                    ble_conn.state = BLE_STATE_READ;
                }
                break;

            case BLE_STATE_WF_RECONN: // wait for the next reading
                {
                    printf("Wait for reconnection-period (%ds) to be elapsed...\n", BLE_RECONN_INTERVAL);
                    sleep(BLE_RECONN_INTERVAL);
                    ble_conn.state = BLE_STATE_NOT_CONNECTED;
                }
                break;

            case BLE_STATE_WF_READ: // wait for the next reading
                {
                    printf("Wait for reading-period (%ds) to be elapsed...\n", BLE_READ_INTERVAL);
                    sleep(BLE_READ_INTERVAL);
                    ble_conn.state = BLE_STATE_READ;
                }
                break;

            case BLE_STATE_READ:
                {
                    double_val = 0;

                    ret = ble_get_float("Temp", &double_val, TIMEOUT);
                    printf("ret: %d, sensor value: %lf\n", ret, double_val);
                    printf("--------------------------------------------------------------------------\n");

                    if (ret < 0) {
                        ble_conn.rw_retry_cnt ++;

                        if (ble_conn.rw_retry_cnt == BLE_RW_RETRYCNT) {
                            ble_conn.state = BLE_STATE_DISCONNECTED; // time to disconnect
                            ble_conn.rw_retry_cnt = 0; // new retrials
                        }
                        else
                            ble_conn.state = BLE_STATE_WF_READ; // try still reading
                    }
                    else { // succesfull reading
                        *temperature = (float) double_val;
                        ble_conn.state = BLE_STATE_WF_READ;
                    }
                }
                break;

            case BLE_STATE_DISCONNECTED: // wait for the next reading
                {
                    printf("Disconnect\n");
                    ble_disconnect();
                    ble_conn.state = BLE_STATE_WF_RECONN;
                }
                break;

        }
    }
}

void *ble_temp_update_thread( void *ptr )
{
    char *ble_addr = ptr;
    update_loop(ble_addr, &ble_temp);
    return NULL; // never reached
}

void start_ble_temp_update_thread(const char *ble_addr)
{
    pthread_create(&ble_update_thread, NULL, ble_temp_update_thread, (void *) ble_addr);
}

void stop_ble_temp_update_thread(void)
{
    pthread_kill(ble_update_thread, SIGKILL);
}
