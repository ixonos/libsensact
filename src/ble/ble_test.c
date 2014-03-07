#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "ble.h"

#define TIMEOUT 2 // sec

int main(int argc, const char* argv[]) {

    int i, ret, paramloc = 0,found = 0;
    double double_val1 = 0, double_val2= 0, double_val3= 0, time_stamp = 0, first_time_stamp = 0;
    char buff[100];

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
    ret = discoverServices();
    printf("--------------------------------ret: %d, discoverServices--------------------------------\n\n", ret);

    ret = discoverCharacteristics();
    printf("--------------------------------ret: %d, discoverCharacteristics--------------------------------\n\n", ret);
    //sleep(3);

//*********************************************************
/*
    memset(buff, 0, sizeof(buff));
    ret = ble_get_string("HWrev", &buff[0], TIMEOUT, &time_stamp);
    printf("ret: %d, Firmware revision: %s\n", ret, buff);

    memset(buff, 0, sizeof(buff));
    ret = ble_get_string("SWrev", &buff[0], TIMEOUT, &time_stamp);
    printf("ret: %d, Software revision: %s\n", ret, buff);

    // run tests
    for (i = 0; i < 10; i++)
    {
	printf("********************************************************************\n");
	ret = ble_get_float("Temp", &double_val1, NULL, NULL, TIMEOUT, &time_stamp);
	printf("ret: %d, sensor value: %lf\n", ret, double_val1);
	sleep(1);
    }
*/
/*
    double_val1 = 0;
    for (i = 0; i < 10; i++)
    {
	printf("********************************************************************\n");
	ret = ble_get_float("humidity", &double_val1, NULL, NULL,TIMEOUT, &time_stamp);
	printf("ret: %d, sensor value: %lf\n", ret, double_val1);
	sleep(3);
    }
*/
/*
    unsigned int interval_us = 50000; // measurement interval in micro seconds
    for (i = 0, double_val1 = 0; i < 100; i++)
    {
        //printf("********************************************************************\n");
	ret = ble_get_float("gyroscope", &double_val1, NULL, NULL, TIMEOUT, &time_stamp);

	if (ret < 0) {
	    printf("gyroscope test FAILED\n");
	    return 1;
	}
	if (!first_time_stamp)
	    first_time_stamp = time_stamp;

	if (!(i % 5)) printf("------------------ gyroZangle: %lf  ---------------\n", double_val1);
	usleep(interval_us);
    }

    printf("------------------ gyro measurement test-time: %lf [seconds] ---------------\n", time_stamp - first_time_stamp);
*/
    double_val1 = 0;
    double_val2= 0;
    double_val3= 0;

    for (i = 0; i < 20; i++)
    {
	printf("********************************************************************\n");
	ret = ble_get_float("accelero", &double_val1, &double_val2, NULL, TIMEOUT, &time_stamp);
	printf("ret: %d, pitch: %lf, roll: %lf\n", ret, double_val1, double_val2);
	usleep(200000);
    }

/*
    double_val1 = 0;
    for (i = 0; i < 5; i++)
    {
	printf("********************************************************************\n");
	ret = ble_get_float("magnetom", &double_val1, NULL, NULL, TIMEOUT, &time_stamp);
	printf("ret: %d, sensor value: %lf\n", ret, double_val1);
	usleep(100000);
    }

*/
    ble_disconnect();

    return 0;
}
