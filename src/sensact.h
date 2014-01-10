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

#ifndef SENSACT_H
#define SENSACT_H

enum connection_t
{
    USB,
    ETHERCAT,
    CAN,
    PROFIBUS,
    PROFINET,
    BLUETOOTH,
    WIFI,
    I2C,
    SPI,
};

struct device_t
{
    const char *name;
    const char *description;
    const enum connection_t connection;

    /* USB options */
    const int vid;
    const int pid;
    const int endpoint;

    /* ETHERCAT options */
    /* CAN options */
    /* PROFIBUS options */
    /* PROFINET options */
    /* BLUETOOTH options */
    /* WIFI options */

};

extern char *sa_error;

int sa_register_devices(struct device_t *devices); // Register devices

int sa_connect(char *name);    // Connect to device with name
int sa_disconnect(int device); // Disconnect device
int sa_reconnect(int device);  // Reconnect to device

int sa_get_char(int device, char *name, char *value, int timeout);   // 8 bit integer value
int sa_get_short(int device, char *name, short *value, int timeout); // 16 bit integer value
int sa_get_int(int device, char *name, int *value, int timeout);     // 32 bit integer value
int sa_get_float(int device, char *name, float *value, int timeout); // 32 bit float value
int sa_get_data(int device, char *name, void *data, int *data_size, int timeout); // Up to 1024 bytes of data

int sa_set_char(int device, char *name, char value, int timeout);    // 8 bit integer value
int sa_set_short(int device, char *name, short value, int timeout);  // 16 bit integer value
int sa_set_int(int device, char *name, int value, int timeout);      // 32 bit integer value
int sa_set_float(int device, char *name, float value, int timeout);  // 32 bit float value
int sa_set_data(int device, char *name, void *data, int data_size, int timeout); // Up to 1024 bytes of data

#endif
