#include <errno.h>
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
#define BLE_READ_INTERVAL  100000 // micro seconds
#define BLE_SCAN_TIME      20 // seconds

//#define BLE_RECONN_RETRYCNT   10
#define BLE_RECONN_INTERVAL  2 // seconds

float ble_temp = 0; // temperature
float ble_pitch = 0; // pitch-angle
float ble_roll = 0; // roll-angle

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
    unsigned int rw_retry_cnt;
} BLE_CONN_T;

void control_am335x_user_led0(unsigned char flag)
{
    FILE *fd;

    // control user led 0
    if (flag == 0) // led OFF
    {
        fd = fopen("/sys/class/leds/evmsk\:green\:usr0/brightness","rw+");
        if (fd)
        {
            fwrite("0", 1, 1, fd); // OFF
            fclose(fd);
        }
        //else syslog(LOG_ERR, "cannot open led-device");
    }
    if (flag == 1)  // led ON
    {
        fd = fopen("/sys/class/leds/evmsk\:green\:usr0/trigger","rw+");
        if (fd)
        {
            fwrite("none", 1, 4, fd); // blinking off
            fclose(fd);
        }

        fd = fopen("/sys/class/leds/evmsk\:green\:usr0/brightness","rw+");
        if (fd)
        {
            fwrite("1", 1, 1, fd); // ON
            fclose(fd);
        }
        //else syslog(LOG_ERR, "cannot open led-device");
    }
    if (flag == 2) // blink the led
    {
        //echo 100 >> /sys/class/leds/am335x\:EVM_SK\:usr0/delay_off
        //echo 100 >> /sys/class/leds/am335x\:EVM_SK\:usr0/delay_on
        // default: 1 sec ON/1 sec OFF
        fd = fopen("/sys/class/leds/evmsk\:green\:usr0/trigger","rw+");
        if (fd)
        {
            fwrite("timer", 1, 5, fd);
            fclose(fd);
        }
        //else syslog(LOG_ERR, "cannot open led-device");	
    }

    return;
}

int monitor_advertisements(int hciSocket, unsigned int scan_time, char *ble_addr)
{
    fd_set rfds;
    struct timeval tv;
    int result = -1;
    unsigned char hciEventBuf[HCI_MAX_EVENT_SIZE];
    int len;
    evt_le_meta_event *leMetaEvent;
    le_advertising_info *leAdvertisingInfo;
    char adv_ble_addr[18]; // advertised ble-address
    struct hci_filter of, nf; // old/new filters
    socklen_t olen;

    olen = sizeof(of);

    if (getsockopt(hciSocket, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
        printf("Could not get socket options\n");
        return -1;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
        printf("Could not set socket options\n");
        return -1;
    }

    FD_ZERO(&rfds);
    FD_SET(hciSocket, &rfds);
    tv.tv_sec = scan_time; // wait for poll_time seconds for advertisement event
    tv.tv_usec = 0;

    result = select(hciSocket + 1, &rfds, NULL, NULL, &tv);

    if (result <= 0) {
        if ( (result == -1) || (result == 0) ) {
            printf("hci-socket read error: %s\n", (result == -1) ? strerror(errno) : "timeout");
            syslog(LOG_ERR, "hci-socket read error: %s\n", (result == -1) ? strerror(errno) : "timeout");
        }
        else {
            printf("hci-socket read error: %s\n", "undefined");
            syslog(LOG_ERR, "hci-socket read error: %s\n", "undefined");
        }
        return -1;
    }

    // read received event
    len = read(hciSocket, hciEventBuf, sizeof(hciEventBuf));
    leMetaEvent = (evt_le_meta_event *)(hciEventBuf + (1 + HCI_EVENT_HDR_SIZE));
    len -= (1 + HCI_EVENT_HDR_SIZE);

    if ( (leMetaEvent->subevent != 0x02) || (len < 0) )
        return -1;

    leAdvertisingInfo = (le_advertising_info *)(leMetaEvent->data + 1);
    ba2str(&leAdvertisingInfo->bdaddr, adv_ble_addr);
    syslog(LOG_ERR, "advertisement event: ble-addr = %s", adv_ble_addr);
    setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    if (ble_addr != NULL)
        strncpy(ble_addr, adv_ble_addr, sizeof(adv_ble_addr));

    return 0;
}

int ble_scan(char *ble_adv_addr)
{
    int dd, err, adv_err, dev_id;
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
        return -1;
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
    adv_err = monitor_advertisements(dd, BLE_SCAN_TIME, ble_adv_addr);
    //sleep(BLE_SCAN_TIME);

    // End scan
    err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 1000);
    if (err < 0 || adv_err < 0)
    {
        perror("Scanning or Disable scan failed");
        goto error;
    }

    goto success;

success:
    hci_close_dev(dd);
    return 0;

error:
    hci_close_dev(dd);
    return -1;
}

// This demo-program is simple way to adapt ble-interface to sensact-platform

void update_loop(char *ble_addr, float *pitch, float *roll, float *temperature)
{

    // There is NO direct function call-API for sensact-server to retrieve BLE-values from libsensact yet,
    // but this separately executable program retrieves BLE-sensor parameters.
    // Sensor-values are returned to upper level components (sensact-server, IxGui) as parameters of the update_loop().

    // This function will handle connection-establishment (connect/reconnect) and reading of the supported sensor-values:
    // - temperature
    // - pitch, roll (calculated by accelerometer)

    // This function will called once, so it should be safe to do ble-specific initializations here

    // In endless-loop it starts to follow next procedure:
    //     BLE_STATE_NOT_CONNECTED -> BLE_STATE_CONNECTED -> BLE_STATE_READ ->
    //     BLE_STATE_WF_READ -> BLE_STATE_READ ->  BLE_STATE_WF_READ -> BLE_STATE_READ -> BLE_STATE_WF_READ -> BLE_STATE_READ ...
    // If problems in reading occur, after BLE_RW_RETRYCNT-retrials:
    //     BLE_STATE_DISCONNECTED -> BLE_STATE_WF_RECONN -> BLE_STATE_NOT_CONNECTED -> BLE_STATE_CONNECTED -> BLE_STATE_READ ...


    struct ble_conn_t ble_conn;
    int ret = -1;
    double tmp_pitch, tmp_roll, tmp_yaw, tmp_temperature;
    char ble_adv_addr[18]; // advertisement address
    double time_stamp = 0;

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

                    // LE scan before connect. So, ble-connect is done in the right moment, when BLE-sensortag is advertising.
		    // Fixes also "no route to host"-error, which sometimes occurs.
                    memset(ble_adv_addr, 0, sizeof(ble_adv_addr));
                    control_am335x_user_led0(2); // led blinking
                    ret = ble_scan(ble_adv_addr);

                    if (ret >= 0)
                    {
                        if (!strcmp(ble_conn.ble_addr, ble_adv_addr))
                            ret = ble_connect(ble_conn.ble_addr); // advertised address match with address given by parameter to update_loop
                        else 
                            ret = -1;
                    }

                    if (ret < 0) // connection establishment failed
                    {
                        ble_conn.state = BLE_STATE_WF_RECONN;
                        control_am335x_user_led0(0); // led off
                    }
                    else
                    {
                        ble_conn.state = BLE_STATE_CONNECTED;
                        control_am335x_user_led0(1); // led on
                    }
                }
                break;

            case BLE_STATE_CONNECTED:
                {
                    printf("BLE connected\n");
		    syslog(LOG_ERR, "BLE connected\n");

                    printf("ret: %d, discoverCharacteristics\n\n", ret);
                    ret = discoverCharacteristics(); // read characteristics (for getting proper UUID-characteristics handle mapping)
                    if (ret < 0) // connection establishment failed
                    {
                        ble_conn.state = BLE_STATE_WF_RECONN;
                        control_am335x_user_led0(0); // led off
                    }
                    else
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
                    //printf("Wait for reading-period (%d ms) to be elapsed...\n", BLE_READ_INTERVAL/1000);
                    usleep(BLE_READ_INTERVAL);
                    ble_conn.state = BLE_STATE_READ;
                }
                break;

            case BLE_STATE_READ:
                {
                    tmp_pitch = 0, tmp_roll = 0, /*tmp_yaw = 0,*/ tmp_temperature = 0;

                     ret = ble_get_float("accelero", &tmp_pitch, &tmp_roll, NULL, TIMEOUT, &time_stamp);
                     if (ret >= 0)
                         ret = ble_get_float("Temp", &tmp_temperature, NULL, NULL, TIMEOUT, &time_stamp);
                    //else if (ret >= 0)
                    //    ret = ble_get_float("gyroscope", &tmp_yaw, NULL, NULL, TIMEOUT, &time_stamp);

                    //printf("ret: %d, ble-accelerometer: pitch: %lf°, roll: %lf°\n", ret, tmp_pitch, tmp_roll);
                    //printf("ret: %d, ble-temperature: %lf°C\n", ret, tmp_temperature);
                    //printf("ret: %d, ble-gyroscope: yaw= %lf°\n", ret, tmp_yaw);
                    //printf("--------------------------------------------------------------------------\n");

                    if (ret < 0) {
                        ble_conn.rw_retry_cnt++;

                        if (ble_conn.rw_retry_cnt >= BLE_RW_RETRYCNT) {
                            ble_conn.state = BLE_STATE_DISCONNECTED; // time to disconnect
                            ble_conn.rw_retry_cnt = 0; // new retrials
                        }
                        else
                            ble_conn.state = BLE_STATE_WF_READ; // try still reading the sensor values
                    }
                    else { // succesfull reading
                        *pitch = (float) tmp_pitch; // update sensor value
                        *roll = (float) tmp_roll; // update sensor value
                        *temperature = (float) tmp_temperature; // update sensor value
                        ble_conn.state = BLE_STATE_WF_READ;
                        ble_conn.rw_retry_cnt = 0; // new retrials
                    }
                }
                break;

            case BLE_STATE_DISCONNECTED: // wait for the next reading
                {
                    ble_disconnect();
                    control_am335x_user_led0(0); // led off
                    printf("BLE disconnect\n");
                    syslog(LOG_ERR, "BLE disconnect\n");
                    ble_conn.state = BLE_STATE_WF_RECONN;
                }
                break;

            default:
                {
                    printf("FATAL: wrong state in update_loop\n");
                    syslog(LOG_ERR, "FATAL: wrong state in update_loop\n");
                    ble_disconnect();
                    control_am335x_user_led0(0); // led off
                    ble_conn.state = BLE_STATE_WF_RECONN;
                }
                break;
        }
    }
}

void *ble_temp_update_thread( void *ptr )
{
    char *ble_addr = ptr;
    update_loop(ble_addr, &ble_pitch, &ble_roll, &ble_temp);
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
