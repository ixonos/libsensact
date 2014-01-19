#include <syslog.h>
#include <stdio.h>
#include <string.h>
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

#define TIMEOUT 2 // sec

int main(int argc, const char* argv[]) {

    int i, ret, paramloc = 0,found = 0;
    double double_val;

    for(i=0; i<argc;i++) {
        if(strcmp(argv[i], "-a") == 0 && !found) {
            found = 1;
            paramloc = i+1;
            if(!ble_validate_address(argv[paramloc])) {
                fprintf(stderr, "ble-address is not valid: %s\n", argv[paramloc]);
                return 1;
            }
        }
    }

    if(!found) {
        fprintf(stderr, "Usage: %s -a BLE_ADDRESS\n", argv[0]);
        return 0;
    }

    openlog("l2cap-ble", LOG_PID, LOG_DAEMON);
    syslog(LOG_ERR, "l2cap-ble starting");

    ble_connect((char *)argv[paramloc]);

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
