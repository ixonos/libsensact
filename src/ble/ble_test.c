#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include "ble.h"

/*unsigned char encoded_request[][8] = { 	
				{0x3, ATT_OP_READ_REQ, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read device name
				{0x4, ATT_OP_WRITE_CMD, 0x29, 0x00, 0x01, 0xFF, 0xFF, 0}, // enable IR-temperature sensor
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0}, // read IR-temperature
				{0x3, ATT_OP_READ_REQ, 0x25, 0x00, 0xFF, 0xFF, 0xFF, 0} // read IR-temperature				
		   	   };
*/

// BLE data
char ble_addr[18] = "BC:6A:29:AB:41:36";

#define TIMEOUT 2 // sec


int main(int argc, const char* argv[]) {

    int i, ret;
    double double_val;

    openlog("l2cap-ble", LOG_PID, LOG_DAEMON);
    syslog(LOG_ERR, "l2cap-ble starting");

    ble_connect(ble_addr);

//*********************************************************
//discoverServices();
//sleep(3);
//*********************************************************


    // run tests
    for (i = 0; i < 10; i++)
    {
	printf("********************************************************************\n");
	ret = ble_get_float("Temp", &double_val, TIMEOUT);
	printf("ret: %d, sensor value: %lf\n", ret, double_val);
	sleep(3);
    }
/*
// run tests
    for (i = 0; i < sizeof(encoded_request)/sizeof(encoded_request[0]); i++)
    {
	printf("********************************************************************\n");
	ble_write_l2cap_socket(&encoded_request[i][1], encoded_request[i][0], 10);

	if (encoded_request[i][1] == ATT_OP_WRITE_CMD) // no response for enabling-command
	    continue;
	else{
	    ble_read_l2cap_socket(l2capSockBuf, sizeof(l2capSockBuf), 10);
	    if (encoded_request[i][2] == 0x25)
		TI_sensortag_temperature(&l2capSockBuf[1]);
	}
	
	sleep(3);
    }
*/
    ble_disconnect();

    return 0;
}
