/*
 * Copyright (c) 2013, Ixonos Denmark ApS
 * Copyright (c) 2013, Martin Lund
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
#include <stdbool.h>
#include <string.h>
#include "sensact.h"
#include "session.h"
#include "packet.h"
#include "debug.h"

static struct device_t *device_list;

char error[PACKET_VALUE_MAX_SIZE];
char *sa_error;

void init(void)
{
    int i;

    // Initialize session structures
    for (i=0; i<MAX_SESSIONS; i++)
        session[i].allocated = false;
}

int register_devices(struct device_t *devices)
{
    device_list = devices;

    return 0;
}

int connect(char *name)
{
    int i=0;
    bool session_available=false;
    bool device_found=false;
    struct device_t *device = device_list;

    debug_printf("Connecting...\n");

    // Find a free session entry (i)
    for (i=0; i<MAX_SESSIONS; i++)
    {
        if (session[i].allocated == false)
        {
            session_available=true;
            break;
        }
    }

    // Return error if no session can be allocated
    if (session_available == false)
    {
        printf("Error: Too many active sessions!\n");
        return -1;
    }

    // Look up device name in list of supported devices
    while (device->name != NULL)
    {
        if (strcmp(device->name, name) == 0)
        {
            device_found = true;
            break;
        }
        device++;
    }

    if (!device_found)
    {
        printf("Error: Device %s not found!\n", name);
        return -1;
    }

    // Initialize session
    session[i].device = device;

    switch (device->connection)
    {
        int status;
        case USB:
        // Call usb_connect()
        status = usb_connect(&session[i].usb.device,
                session[i].device->vid,
                session[i].device->pid,
                session[i].device->endpoint);
        if (status < 0)
            return -1;

        // Map functions
        session[i].disconnect = usb_disconnect;
        session[i].reconnect = usb_reconnect;
        session[i].write = usb_write;
        session[i].read = usb_read;
        break;
        case ETHERCAT:
        case CAN:
        case PROFIBUS:
        case BLUETOOTH:
        case WIFI:
        default:
        break;
    }

    // Return session handle
    return i;
}

int disconnect(int handle)
{
    session[handle].disconnect(handle);
    return 0;
}

int reconnect(int handle)
{
    return session[handle].reconnect(handle);
}

int send_command(
        int handle,
        int command,
        char *name,
        void *get_value,
        int *get_value_size,
        void *set_value,
        int set_value_size,
        int timeout)
{
    struct request_packet_t request_packet = {};
    struct response_packet_t response_packet = {};
    int packet_length;
    unsigned short id;
    int ret;
    char data[PACKET_VALUE_MAX_SIZE];

    // Create request packet
    packet_length = create_request_packet(&request_packet, command, name, set_value, set_value_size, &id);
    debug_printf("packet_length=%d\n", packet_length);

    if (packet_length < 0)
        return -1;

    debug_printf("Packet ID: %d\n", id); // Debug

    // Send packet
    ret = session[handle].write(handle, (char *) &request_packet, packet_length, timeout);
    if (ret < 0 )
        return -1;

    // Receive response header
    ret = session[handle].read(handle, (char *) &response_packet, PACKET_RSP_HEADER_SIZE, timeout);
    if (ret < 0 )
        return -1;

    // Verify response header
    if (verify_response_packet(&response_packet, id) == 0)
    {
        if (response_packet.data_length > 0)
        {
            // Receive response payload
            ret = session[handle].read(handle, (char *) &data, response_packet.data_length, timeout);
            if (ret < 0 )
                return -1;

            if (response_packet.response_code == RSP_OK)
            {
                // Extract value from response message (if get request)
                if (get_value != NULL)
                    memcpy(get_value, &data, response_packet.data_length);
            }
            else
            {
                // Error occured, payload is error message
                memcpy(&error, &data, response_packet.data_length);
                error[response_packet.data_length] = 0;
                sa_error = (char *) &error;
                return -1;
            }
        }

        // Return value size in case of getting data
        if (command == GET_DATA)
            *get_value_size = response_packet.data_length;

    } else
        return -1;

    return 0;
}

/* Get functions */

int get_char(int handle, char *name, char *value, int timeout)
{
    return send_command(handle, GET_CHAR, name, (void *) value, NULL, NULL, 0, timeout);
}

int get_short(int handle, char *name, short *value, int timeout)
{
    return send_command(handle, GET_SHORT, name, (void *) value, NULL, NULL, 0, timeout);
}

int get_int(int handle, char *name, int *value, int timeout)
{
    return send_command(handle, GET_INT, name, (void *) value, NULL, NULL, 0, timeout);
}

int get_float(int handle, char *name, float *value, int timeout)
{
    return send_command(handle, GET_FLOAT, name, (void *) value, NULL, NULL, 0, timeout);
}

int get_data(int handle, char *name, void *data, int *data_size, int timeout)
{
    return send_command(handle, GET_DATA, name, data, data_size, NULL, 0, timeout);
}

/* Set functions */

int set_char(int handle, char *name, char value, int timeout)
{
    return send_command(handle, SET_CHAR, name, NULL, NULL, (void *) &value, sizeof(char), timeout);
}

int set_short(int handle, char *name, short value, int timeout)
{
    return send_command(handle, SET_SHORT, name, NULL, NULL, (void *) &value, sizeof(short), timeout);
}

int set_int(int handle, char *name, int value, int timeout)
{
    return send_command(handle, SET_INT, name, NULL, NULL, (void *) &value, sizeof(int), timeout);
}

int set_float(int handle, char *name, float value, int timeout)
{
    return send_command(handle, SET_FLOAT, name, NULL, NULL, (void *) &value, sizeof(float), timeout);
}

int set_data(int handle, char *name, void *data, int data_size, int timeout)
{
    if (data_size > PACKET_VALUE_MAX_SIZE)
    {
        sa_error = "Error: Size of data must not exceed 1024 bytes";
        return -1;
    }
    return send_command(handle, SET_DATA, name, NULL, NULL, data, data_size, timeout);
}
