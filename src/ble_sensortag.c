#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb.h>
#include "sensact/debug.h"
#include "sensact/session.h"
#include "sensact/ble_sensortag.h"
#include "sensact/ble_sensortag_config.h"
#include "../ble/ble_update.h"

int ble_sensortag_connect(int device, void *config)
{
    struct ble_sensortag_config_t *sensortag_config = config;
    start_ble_temp_update_thread(sensortag_config->ble_address);
    return SA_OK;
}

int ble_sensortag_disconnect(int device)
{
    stop_ble_temp_update_thread();
    return SA_OK;
}

int ble_sensortag_get_float(int device, char *name, float *value, int timeout)
{
    *value = ble_temp;
    return SA_OK;
}

struct sa_backend_t ble_sensortag_backend =
{
    .name = "ble_sensortag",
    .connect = ble_sensortag_connect,
    .disconnect = ble_sensortag_disconnect,
    .get_float = ble_sensortag_get_float,
};
