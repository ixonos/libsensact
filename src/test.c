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
#include <unistd.h>
#include <string.h>
#include "sensact.h"

//#define SENSHUB_TEST
#define DUMMY_TEST
//#define NFC_TEST

#define TIMEOUT 100 // ms

int main(void)
{
    int handle;
    int i=1;

#ifdef SENSHUB_TEST

    int roll, pitch, yaw, altitude;
    float ambTemp, pressure, refPressure = 101325.0f, humidity;

    handle = connect("senshub0");

    set_float(handle, "refPressure", refPressure, TIMEOUT);

    while (i--)
    {
        get_int(handle, "roll", &roll, TIMEOUT);
        printf("roll:     %03d\n", roll);

        get_int(handle, "pitch", &pitch, TIMEOUT);
        printf("pitch:    %03d\n", pitch);

        get_int(handle, "yaw", &yaw, TIMEOUT);
        printf("yaw:      %03d\n", yaw);

        get_float(handle, "objTemp", &ambTemp, TIMEOUT);
        printf("temp:     %2.2f\n", ambTemp);

        get_float(handle, "pressure", &pressure, TIMEOUT);
        printf("presure:  %2.2f Pa\n", pressure);

        get_int(handle, "altitude", &altitude, TIMEOUT);
        printf("altitude: %d m\n", altitude);

        get_float(handle, "humidity", &humidity, TIMEOUT);
        printf("humidity: %2.2f\n", humidity);

        sleep(1);
    }

    disconnect(handle);

#endif // SENSHUB_TEST

#ifdef DUMMY_TEST

    char char0 = 0;
    short short0;
    int int0;
    float float0;
    char data0[1024] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
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
    int size;

    handle = connect("dummy0");

    while (i--)
    {
        set_char(handle, "char0", 42, TIMEOUT);
        get_char(handle, "char0", &char0, TIMEOUT);
        printf("char0 = %d\n", char0);

        set_short(handle, "short0", 4343, TIMEOUT);
        get_short(handle, "short0", &short0, TIMEOUT);
        printf("short0 = %d\n", short0);

        set_int(handle, "int0", 444444, TIMEOUT);
        get_int(handle, "int0", &int0, TIMEOUT);
        printf("int0 = %d\n", int0);

        set_float(handle, "float0", 45.45, TIMEOUT);
        get_float(handle, "float0", &float0, TIMEOUT);
        printf("float0: %2.2f\n", float0);

        set_data(handle, "data0", &data0, strlen(data0)+1, TIMEOUT);
        get_data(handle, "data0", &data0_rcv, &size, TIMEOUT);
        printf("data0: %s\n", (char *) &data0_rcv);
        printf("size=%d\n", size);

        // Error test
        if (get_char(handle, "char1", &char0, TIMEOUT) == 0)
            printf("char0 = %d\n", char0);
        else
            printf("Error: %s\n", sa_error);
    }

    disconnect(handle);

#endif // DUMMY_TEST

#ifdef NFC_TEST

    char char0;
    char record0[256] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                        "0123456789012345678901234567890123456789012345678901234";
    char record0_rcv[1024];
    int size;

    handle = connect("nfc0");

    while (i--)
    {
        set_data(handle, "record0", &record0, strlen(record0)+1, TIMEOUT);
        get_data(handle, "record0", &record0_rcv, &size, TIMEOUT);
        printf("record0: %s\n", (char *) &record0_rcv);
        printf("size=%d\n", size);
    }

    disconnect(handle);

#endif // NFC_TEST

    return 0;
}
