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
#include <libusb.h>
#include "sensact/plugins/usb-config.h"
#include "sensact/session.h"
#include "sensact/debug.h"
#include "sensact/plugin.h"
#include "sensact/sensact.h"

struct usb_data_t
{
    struct libusb_device_handle *device;
};

int usb_connect(int device, void *config)
{
    struct usb_config_t *usb_config = config;
    struct usb_data_t *usb_data;

    usb_data = malloc(sizeof(struct usb_data_t));
    session[device].data = usb_data;

    libusb_device_handle *usb_device;

    libusb_init(NULL);

    usb_device = libusb_open_device_with_vid_pid(NULL, usb_config->vid, usb_config->pid);

    if (usb_device != NULL)
    {
        libusb_detach_kernel_driver(usb_device, 0);

        int claimed = libusb_claim_interface(usb_device, 0);

        if (claimed == 0)
        {
            debug_printf("Connected to USB device (vid=%X,pid=%X)\n", usb_config->vid, usb_config->pid);

            // Store USB device handle for session
            usb_data->device = usb_device;

            return SA_OK;
        }

        libusb_close(usb_device);
    }

    printf("Error: Could not connect USB device (vid=%X,pid=%X)\n", usb_config->vid, usb_config->pid);

    return SA_ERROR;
}

int usb_disconnect(int device)
{
    struct usb_data_t *usb_data = session[device].data;

    debug_printf("Disconnecting USB device...\n");

    // Release USB interface
    libusb_release_interface(usb_data->device, 0);
    libusb_close(usb_data->device);

    free(usb_data);

    session[device].connected = false;

    return SA_OK;
}

static int usb_reconnect(int device)
{
    struct usb_config_t *usb_config = session[device].device->config;
    struct usb_data_t *usb_data = session[device].data;
    int ret;

    debug_printf("Reconnecting...\n");

    // Wait a short while to let USB device settle a bit
    // usleep(100000); // 100 ms

    // Only disconnect if connected
    if (session[device].connected)
        ret = usb_disconnect(device);

    ret = usb_connect(device,session[device].device->config);

    if (ret == SA_OK)
        session[device].connected = true;

    return ret;
}

int usb_write(int device, char *data, int length, int timeout)
{
    struct usb_config_t *usb_config = session[device].device->config;
    struct usb_data_t *usb_data = session[device].data;
    int count;
    int ret = LIBUSB_ERROR_OTHER;
    int i;

    // Debug
    debug_printf("Sending data,  %d bytes (USB): ", length);
    for (i=0; i<length; i++)
        debug_printf_raw("0x%x ", (unsigned char) data[i]);
    debug_printf_raw("\n");

    // Write bulk message
    if (session[device].connected)
        ret = libusb_bulk_transfer(usb_data->device,
                                   usb_config->endpoint | LIBUSB_ENDPOINT_OUT,
                                   (unsigned char *) data, length,
                                   &count, timeout);

    // If device disconnected try reconnect once
    if ((ret == LIBUSB_ERROR_NO_DEVICE) || (ret == LIBUSB_ERROR_IO) || !session[device].connected)
        ret = usb_reconnect(device);

    if (ret == LIBUSB_SUCCESS)
        return count;
    else
    {
        printf("Error: USB write failed (%d) %s\n", ret, libusb_error_name(ret));
        return -1;
    }
}

int usb_read(int device, char *data, int length, int timeout)
{
    struct usb_config_t *usb_config = session[device].device->config;
    struct usb_data_t *usb_data = session[device].data;
    int count;
    int ret = LIBUSB_ERROR_OTHER;
    int i;

    // Read bulk message
    if (session[device].connected)
        ret = libusb_bulk_transfer(usb_data->device,
                                   usb_config->endpoint | LIBUSB_ENDPOINT_IN,
                                   (unsigned char *) data, length,
                                   &count, timeout);

    // If device disconnected try reconnect once
    if ((ret == LIBUSB_ERROR_NO_DEVICE) || (ret == LIBUSB_ERROR_IO) || !session[device].connected)
        ret = usb_reconnect(device);

    // Debug
    debug_printf("Received data, %d bytes (USB): ", count);
    for (i=0; i<count; i++)
        debug_printf_raw("0x%x ", (unsigned char) data[i]);
    debug_printf_raw("\n");

    if (ret == LIBUSB_SUCCESS)
        return count;
    else
    {
        printf("Error: USB read failed (%d) %s\n", ret, libusb_error_name(ret));
        return -1;
    }
}

struct sa_backend_t usb_backend =
{
    .name = "usb",
    .connect = usb_connect,
    .disconnect = usb_disconnect,
    .read = usb_read,
    .write = usb_write,
};


// Plugin configuration
struct sa_plugin_t usb =
{
    .description = "Sensact USB plugin",
    .version = "0.1",
    .author = "Martin Lund",
    .license = "BSD-3",
    .backend = &usb_backend,
};

sa_plugin_register(usb);
