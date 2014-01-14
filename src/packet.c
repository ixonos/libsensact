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
#include <string.h>
#include "sensact/packet.h"
#include "sensact/debug.h"

#ifdef SENSACT_CLIENT
unsigned short packet_counter = 0;


int create_request_packet(
        struct request_packet_t *packet,
        int command,
        char *name,
        void *set_value,
        int set_value_length,
        unsigned short *id)
{
    debug_printf("Creating request packet...\n");

    // Verify length of name
    if (strlen(name) > PACKET_NAME_MAX_SIZE)
    {
        printf("Error: Lenth of name must not exceed %d bytes\n", PACKET_NAME_MAX_SIZE);
        return -1;
    }

    // Verify length of value
    if (set_value_length > PACKET_VALUE_MAX_SIZE)
    {
        printf("Error: Length of value must not exceed %d bytes\n", PACKET_VALUE_MAX_SIZE);
        return -1;
    }

    // Create command packet
    packet->prefix = PACKET_PREFIX;
    packet->command_code = command;
    packet->id = packet_counter;

    // Configure name/value lengths and pack data accordingly
    switch (command)
    {
        case GET_CHAR:
        case GET_SHORT:
        case GET_INT:
        case GET_FLOAT:
        case GET_DATA:
            packet->name_length = strlen(name);
            packet->value_length = 0;
            strcpy(packet->data, name);
            break;
        case SET_CHAR:
        case SET_SHORT:
        case SET_INT:
        case SET_FLOAT:
        case SET_DATA:
            packet->name_length = strlen(name);
            packet->value_length = set_value_length;
            strcpy(packet->data, name);
            memcpy(&packet->data[packet->name_length], set_value, set_value_length);
            break;
        default:
            break;
    }

    // Return packet ID
    *id = packet_counter;

    // Increase packet counter
    packet_counter++;

    // Return length of request packet
    return PACKET_REQ_HEADER_SIZE + packet->name_length + packet->value_length;
}

int verify_response_packet(struct response_packet_t *packet, unsigned short id)
{
    debug_printf("Verifying response packet...\n");

    // Verify packet prefix
    if (packet->prefix != PACKET_PREFIX)
    {
        printf("Error: Received invalid response packet (invalid prefix)\n");
        return -1;
    }

    // Verify response code
    if ((packet->response_code != RSP_OK) && (packet->response_code != RSP_ERROR))
    {
        printf("Error: Received invalid response packet (invalid response code)\n");
        return -1;
    }

    // Verify response id
    if (packet->id != id)
    {
        printf("Error: Received invalid response packet (invalid response id)\n");
        return -1;
    }

    return 0;
}

#endif // SENSACT_CLIENT


#ifdef SENSACT_SERVER
int verify_request_packet(struct request_packet_t *packet)
{
    debug_printf("Verifying request packet...\n");

    // Verify packet prefix
    if (packet->prefix != PACKET_PREFIX)
    {
        printf("Error: Received invalid request packet (invalid prefix)\n");
        return -1;
    }

    return 0;
}

int decode_request_packet(struct request_packet_t *packet, int *command, char *name, char *value, unsigned short *id)
{
    debug_printf("Decoding request packet:\n");
    debug_printf("packet->prefix=%x\n", packet->prefix);
    debug_printf("packet->command_code=%x\n", packet->command_code);
    debug_printf("packet->id=%x\n", packet->id);
    debug_printf("packet->name_length=%x\n", packet->name_length);
    debug_printf("packet->value_length=%x\n", packet->value_length);

    // Decode command code
    *command = packet->command_code;

    // Decode packet ID
    *id = packet->id;

    // Decode name
    strncpy(name, packet->data, packet->name_length);
    name[packet->name_length] = 0;
    debug_printf("name=%s\n", name);

    // Decode value (if any)
    if (packet->value_length)
        memcpy(value, &packet->data[packet->name_length], packet->value_length);

    return 0;
}

int create_response_packet(struct response_packet_t *packet, int command, char *data, int length, unsigned short id)
{
    debug_printf("Creating response packet...\n");

    // Verify length of data
    if (length > PACKET_VALUE_MAX_SIZE)
    {
        printf("Error: Length of response data must not exceed %d bytes\n", PACKET_VALUE_MAX_SIZE);
        return -1;
    }

    // Create response packet
    packet->prefix = PACKET_PREFIX;
    packet->response_code = command;
    packet->id = id;
    packet->data_length = length;

    if (length > 0)
        memcpy(packet->data, data, length);

    return PACKET_RSP_HEADER_SIZE + length;
}
#endif // SENSACT_SERVER
