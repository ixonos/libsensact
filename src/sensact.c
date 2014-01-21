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
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "sensact/sensact.h"
#include "sensact/session.h"
#include "sensact/packet.h"
#include "sensact/debug.h"
#include "sensact/list.h"
#include "sensact/usb.h"
#include "sensact/ble_sensortag.h"

static int get_char(int device, char *name, char *value, int timeout);
static int get_short(int device, char *name, short *value, int timeout);
static int get_int(int device, char *name, int *value, int timeout);
static int get_float(int device, char *name, float *value, int timeout);
static int get_data(int device, char *name, void *data, int *data_size, int timeout);

static int set_char(int device, char *name, char value, int timeout);
static int set_short(int device, char *name, short value, int timeout);
static int set_int(int device, char *name, int value, int timeout);
static int set_float(int device, char *name, float value, int timeout);
static int set_data(int device, char *name, void *data, int data_size, int timeout);

static struct sa_device_t *device_list;
static list_p backend_list;

char error[PACKET_VALUE_MAX_SIZE];
char *sa_error;

void init(void)
{
    int i;

    // Initialize session structures
    for (i=0; i<MAX_SESSIONS; i++)
        session[i].allocated = false;

    // Create list for backend references
    backend_list = create_list();

    // Register default backend(s)
    if (sa_register_backend(&usb_backend) != SA_OK)
        printf("Error: %s\n", sa_error);

    if (sa_register_backend(&ble_sensortag_backend) != SA_OK)
        printf("Error: %s\n", sa_error);
}

int sa_register_device(struct sa_device_t *device)
{
    // To be imlemented (FIXME)
    return SA_OK;
}

int sa_unregister_device(char *name)
{
    // To be imlemented (FIXME)
    return SA_OK;
}

int sa_register_devices(struct sa_device_t *devices)
{
    // Check that device of same name is not already registered (FIXME)

    device_list = devices;
    return SA_OK;
}

int sa_register_backend(struct sa_backend_t *backend)
{
    struct sa_backend_t *backendp;
    pthread_mutex_lock(&session_mutex);

    // Check that new backend has a name
    if (backend->name == NULL)
    {
        sa_error = "Backend must have a name!";
        pthread_mutex_unlock(&session_mutex);
        return SA_ERROR;
    }

    // Check that backend of same name is not already registered
    list_iter_p iter = list_iterator(backend_list, FRONT);
    while (list_next(iter)!=NULL)
    {
        backendp = (struct sa_backend_t *)list_current(iter);
        if (strcmp(backendp->name, backend->name) == 0)
        {
            sa_error = "Backend already registered";
            pthread_mutex_unlock(&session_mutex);
            return SA_ERROR;
        }
    }

    // Check that backend implements required calls (FIXME)

    // Add backend to backend list
    list_add(backend_list, backend, sizeof(struct sa_backend_t));

    pthread_mutex_unlock(&session_mutex);

    return SA_OK;
}

int sa_unregister_backend(char *name)
{
    // To be implemented (FIXME)
    return SA_OK;
}

int sa_list_backends(char *backends)
{
    struct sa_backend_t *backend;
    pthread_mutex_lock(&session_mutex);

    // Construct space-separated list of backends
    list_iter_p iter = list_iterator(backend_list, FRONT);
    while (list_next(iter)!=NULL)
    {
        backend = (struct sa_backend_t *)list_current(iter);
        sprintf(backends, "'%s' ", backend->name);
    }

    pthread_mutex_unlock(&session_mutex);
    return SA_OK;
}

int sa_connect(char *name)
{
    int i=0;
    bool session_available=false;
    bool device_found=false;
    bool backend_found=false;
    struct sa_device_t *device = device_list;
    struct sa_backend_t *backend;

    debug_printf("Connecting...\n");
    pthread_mutex_lock(&session_mutex);

    // Find a free device session entry (i)
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
        goto error;
    }

    // Find device in list of registered devices
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
        goto error;
    }

    // Map device information into session
    session[i].device = device;

    // Find backend for device
    list_iter_p iter = list_iterator(backend_list, FRONT);
    while (list_next(iter)!=NULL)
    {
        backend = (struct sa_backend_t *)list_current(iter);
        if (strcmp(backend->name, device->backend) == 0)
            backend_found = true;
    }

    if (!backend_found)
    {
        printf("Error: Backend %s not found!\n", device->backend);
        goto error;
    }

    debug_printf("Found backend for %s: %s\n", device->name, backend->name);

    // Backend found - install defined backend calls
    session[i].connect = backend->connect;
    session[i].disconnect = backend->disconnect;

    session[i].read = backend->read;
    session[i].write = backend->write;

    // Install defined get/set backend calls or use defaults which uses read/write calls
    (backend->get_char != NULL) ? (session[i].get_char = backend->get_char) : (session[i].get_char = get_char);
    (backend->set_char != NULL) ? (session[i].set_char = backend->set_char) : (session[i].set_char = set_char);

    (backend->get_short != NULL) ? (session[i].get_short = backend->get_short) : (session[i].get_short = get_short);
    (backend->set_short != NULL) ? (session[i].set_short = backend->set_short) : (session[i].set_short = set_short);

    (backend->get_int != NULL) ? (session[i].get_int = backend->get_int) : (session[i].get_int = get_int);
    (backend->set_int != NULL) ? (session[i].set_int = backend->set_int) : (session[i].set_int = set_int);

    (backend->get_float != NULL) ? (session[i].get_float = backend->get_float) : (session[i].get_float = get_float);
    (backend->set_float != NULL) ? (session[i].set_float = backend->set_float) : (session[i].set_float = set_float);

    (backend->get_data != NULL) ? (session[i].get_data = backend->get_data) : (session[i].get_data = get_data);
    (backend->set_data != NULL) ? (session[i].set_data = backend->set_data) : (session[i].set_data = set_data);

    // Call connect
    int status = session[i].connect(i, device->config);
    if (status != SA_OK)
        goto error;

    session[i].connected = true;

    // Claim session
    session[i].allocated = true;

    pthread_mutex_unlock(&session_mutex);

    // Return session device handle
    return i;

error:
    pthread_mutex_unlock(&session_mutex);
    return -1;
}

int sa_disconnect(int device)
{
    session[device].disconnect(device);

    pthread_mutex_lock(&session_mutex);
    session[device].allocated = false;
    pthread_mutex_unlock(&session_mutex);

    return SA_OK;
}

static int send_command(
        int device,
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
        return SA_ERROR;

    debug_printf("Packet ID: %d\n", id); // Debug

    // Send packet
    ret = session[device].write(device, (char *) &request_packet, packet_length, timeout);
    if (ret < 0 )
        return SA_ERROR;

    // Receive response header
    ret = session[device].read(device, (char *) &response_packet, PACKET_RSP_HEADER_SIZE, timeout);
    if (ret < 0 )
        return SA_ERROR;

    // Verify response header
    if (verify_response_packet(&response_packet, id) == SA_OK)
    {
        if (response_packet.data_length > 0)
        {
            // Receive response payload
            ret = session[device].read(device, (char *) &data, response_packet.data_length, timeout);
            if (ret < 0 )
                return SA_ERROR;

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
                return SA_ERROR;
            }
        }

        // Return value size in case of getting data
        if (command == GET_DATA)
            *get_value_size = response_packet.data_length;

    } else
        return SA_ERROR;

    return SA_OK;
}

// Default get functions (send_command uses read/write functions)

static int get_char(int device, char *name, char *value, int timeout)
{
    return send_command(device, GET_CHAR, name, (void *) value, NULL, NULL, 0, timeout);
}

static int get_short(int device, char *name, short *value, int timeout)
{
    return send_command(device, GET_SHORT, name, (void *) value, NULL, NULL, 0, timeout);
}

static int get_int(int device, char *name, int *value, int timeout)
{
    return send_command(device, GET_INT, name, (void *) value, NULL, NULL, 0, timeout);
}

static int get_float(int device, char *name, float *value, int timeout)
{
    return send_command(device, GET_FLOAT, name, (void *) value, NULL, NULL, 0, timeout);
}

static int get_data(int device, char *name, void *data, int *data_size, int timeout)
{
    return send_command(device, GET_DATA, name, data, data_size, NULL, 0, timeout);
}

// Get functions

int sa_get_char(int device, char *name, char *value, int timeout)
{
    return session[device].get_char(device, name, value, timeout);
}

int sa_get_short(int device, char *name, short *value, int timeout)
{
    return session[device].get_short(device, name, value, timeout);
}

int sa_get_int(int device, char *name, int *value, int timeout)
{
    return session[device].get_int(device, name, value, timeout);
}

int sa_get_float(int device, char *name, float *value, int timeout)
{
    return session[device].get_float(device, name, value, timeout);
}

int sa_get_data(int device, char *name, void *data, int *data_size, int timeout)
{
    return session[device].get_data(device, name, data, data_size, timeout);
}

// Default set functions (send_command uses read/write)

static int set_char(int device, char *name, char value, int timeout)
{
    return send_command(device, SET_CHAR, name, NULL, NULL, (void *) &value, sizeof(char), timeout);
}

static int set_short(int device, char *name, short value, int timeout)
{
    return send_command(device, SET_SHORT, name, NULL, NULL, (void *) &value, sizeof(short), timeout);
}

static int set_int(int device, char *name, int value, int timeout)
{
    return send_command(device, SET_INT, name, NULL, NULL, (void *) &value, sizeof(int), timeout);
}

static int set_float(int device, char *name, float value, int timeout)
{
    return send_command(device, SET_FLOAT, name, NULL, NULL, (void *) &value, sizeof(float), timeout);
}

static int set_data(int device, char *name, void *data, int data_size, int timeout)
{
    if (data_size > PACKET_VALUE_MAX_SIZE)
    {
        sa_error = "Size of data must not exceed 1024 bytes";
        return SA_ERROR;
    }
    return send_command(device, SET_DATA, name, NULL, NULL, data, data_size, timeout);
}

// Set functions

int sa_set_char(int device, char *name, char value, int timeout)
{
    return session[device].set_char(device, name, value, timeout);
}

int sa_set_short(int device, char *name, short value, int timeout)
{
    return session[device].set_short(device, name, value, timeout);
}

int sa_set_int(int device, char *name, int value, int timeout)
{
    return session[device].set_int(device, name, value, timeout);
}

int sa_set_float(int device, char *name, float value, int timeout)
{
    return session[device].set_float(device, name, value, timeout);
}

int sa_set_data(int device, char *name, void *data, int data_size, int timeout)
{
    if (data_size > PACKET_VALUE_MAX_SIZE)
    {
        sa_error = "Size of data must not exceed 1024 bytes";
        return SA_ERROR;
    }
    return session[device].set_data(device, name, data, data_size, timeout);
}
