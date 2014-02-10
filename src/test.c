/*
 * Copyright (c) 2013-2014, Ixonos Denmark ApS
 * Copyright (c) 2013-2014, Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sensact/sensact.h"
#include "sensact/plugins/usb-config.h"

#define TIMEOUT 100 // ms

// Test configuration
#define DUMMY1
#define LOOP_COUNT 100000
#define LOOP_DELAY 100000 // us

// List of supported sensor/actuator devices

struct usb_config_t usb_dummy0_config =
{
      .vid = 0x1CBE,
      .pid = 0x0040,
      .endpoint = 0x1,
};

struct usb_config_t usb_dummy1_config =
{
      .vid = 0x1CBE,
      .pid = 0x0050,
      .endpoint = 0x1,
};

struct sa_device_t devices[] =
{
   {  .name = "dummy0",
      .description = "TI Tiva Launchpad device (dummy)",
      .backend = "usb",
      .config = &usb_dummy0_config },

   {  .name = "dummy1",
      .description = "TI Tiva Launchpad device (dummy)",
      .backend = "usb",
      .config = &usb_dummy1_config },

   { }
};

void dummy_test(int device)
{
    // Test variables/data
    char  char0;
    short short0;
    int   int0;
    float float0;
    char  data0[1024] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "01234567890123456789012";
    char data0_rcv[1024];
    int  data0_size;

    sa_set_char(device, "char0", 42, TIMEOUT);
    sa_get_char(device, "char0", &char0, TIMEOUT);
    printf("char0 = %d\n", char0);

    sa_set_short(device, "short0", 4343, TIMEOUT);
    sa_get_short(device, "short0", &short0, TIMEOUT);
    printf("short0 = %d\n", short0);

    sa_set_int(device, "int0", 444444, TIMEOUT);
    sa_get_int(device, "int0", &int0, TIMEOUT);
    printf("int0 = %d\n", int0);

    sa_set_float(device, "float0", 45.45, TIMEOUT);
    sa_get_float(device, "float0", &float0, TIMEOUT);
    printf("float0: %2.2f\n", float0);

    sa_set_data(device, "data0", &data0, strlen(data0)+1, TIMEOUT);
    sa_get_data(device, "data0", &data0_rcv, &data0_size, TIMEOUT);
    printf("data0: %s\n", (char *) &data0_rcv);
    printf("data0 size=%d\n", data0_size);

    // Error test (char1 not known by device)
    if (sa_get_char(device, "char1", &char0, TIMEOUT) == SA_OK)
        printf("char1 = %d\n", char0);
    else
        printf("Error: %s\n", sa_error);
}


int main(void)
{
    int dummy0, dummy1;
    int loop=LOOP_COUNT;
    char backends[400];

    // Load backend plugins
    sa_plugin_load("usb");

    // Register known devices
    sa_register_devices((struct sa_device_t *) &devices);

    // List available backends
    sa_list_backends((char *) &backends);
    printf("Available sensact communication backends: %s\n", backends);

    // Connect to device(s)
    dummy0 = sa_connect("dummy0");
    if (dummy0 < 0)
        return -1;

#ifdef DUMMY1
    dummy1 = sa_connect("dummy1");
    if (dummy1 < 0)
        return -1;
#endif

    // Run test loop
    while (loop--)
    {
        dummy_test(dummy0);
#ifdef DUMMY1
        dummy_test(dummy1);
#endif
        usleep(LOOP_DELAY);
    }

    // Disconnect from device(s)
    sa_disconnect(dummy0);
#ifdef DUMMY1
    sa_disconnect(dummy1);
#endif

    return 0;
}
