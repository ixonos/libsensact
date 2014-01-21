#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sensact/sensact.h"
#include "sensact/ble_sensortag_config.h"

#define TIMEOUT 100 // ms

/* List of supported sensor/actuator devices */

struct ble_sensortag_config_t ble_sensortag0_config =
{
      .ble_address = "BC:6A:29:C3:3C:79",
};

struct ble_sensortag_config_t ble_sensortag1_config =
{
      .ble_address = "BC:6A:29:AB:41:36",
};

struct sa_device_t devices[] =
{
   {  .name = "ble_sensortag0",
      .description = "TI sensortag 0",
      .backend = "ble_sensortag",
      .config = &ble_sensortag0_config },

   {  .name = "ble_sensortag1",
      .description = "TI sensortag 1",
      .backend = "ble_sensortag",
      .config = &ble_sensortag1_config },

   { }
};

int main(void)
{
    int sensortag0, sensortag1;
    int loop=100000;
    char backends[400];
    float ble_temp;

    // Register known devices
    sa_register_devices((struct sa_device_t *) &devices);

    // List available backends
    sa_list_backends((char *) &backends);
    printf("Available sensact communication backends: %s\n", backends);

    // Connect to device(s)
    sensortag0 = sa_connect("ble_sensortag0");
    if (sensortag0 < 0)
        return -1;

    // Run test loop
    while (loop--)
    {
        sa_get_float(sensortag0, "ble_temp", &ble_temp, TIMEOUT);
        printf("sa_get_float('ble_temp')=%f\n", ble_temp);
        usleep(1000000);
    }

    // Disconnect from device(s)
    sa_disconnect(sensortag0);

    return 0;
}
