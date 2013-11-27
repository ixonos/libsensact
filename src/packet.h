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

#ifndef PACKET_H
#define PACKET_H

/*
 * == The sensact protocol ==
 *
 * A simple protocol for communicating sensor/actuator values (attributes,
 * parameters, etc.).
 *
 * Request packet format (bytes):
 *  packet[0]=prefix (0xAA)
 *  packet[1-2]=16 bit packet ID
 *  packet[3]=command code
 *  packet[4]=length of name (max 256 bytes)
 *  packet[5-6]=length of value (max 1024 bytes)
 *  packet[7-*]=name
 *  packet[*-*]=value
 *
 *
 *  The request packet length is 6 + sizeof(name+value) bytes
 *  Maximum request packet length is 518 bytes
 *
 * Response packet format (bytes):
 *  packet[0]=prefix (0xAA)
 *  packet[1-2]=16 bit packet ID (response)
 *  packet[3]=response code
 *  packet[4-5]=length of response data (max 1024 bytes)
 *  packet[6-*]=data
 *
 *  The response packet length is 5 + sizeof(data) bytes
 *  Maximum response packet length is 261 bytes
 *  A response packet ID must match the request packet ID.
 *
 * command codes = {GET_CHAR, GET_SHORT, GET_INT, GET_FLOAT, GET_DATA,
 *                  SET_CHAR, SET_SHORT, SET_INT, SET_FLOAT, SET_DATA,
 *                  (CONNECT, DISCONNECT)}
 * response codes = {RSP_OK, RSP_ERROR}
 *
 */

#define PACKET_PREFIX 0xAA
#define PACKET_REQ_HEADER_SIZE 7
#define PACKET_RSP_HEADER_SIZE 6
#define PACKET_NAME_MAX_SIZE 256
#define PACKET_VALUE_MAX_SIZE 1024

enum command_t
{
   GET_CHAR,
   GET_SHORT,
   GET_INT,
   GET_FLOAT,
   GET_DATA,
   SET_CHAR,
   SET_SHORT,
   SET_INT,
   SET_FLOAT,
   SET_DATA,
};

enum response_t
{
   RSP_OK,
   RSP_ERROR,
};

struct __attribute__((__packed__)) request_packet_t
{
   unsigned char prefix;
   unsigned short id;
   unsigned char command_code;
   unsigned char name_length;
   unsigned short value_length;
   char data[PACKET_NAME_MAX_SIZE+PACKET_VALUE_MAX_SIZE];
};

struct __attribute__((__packed__)) response_packet_t
{
   unsigned char prefix;
   unsigned short id;
   unsigned char response_code;
   unsigned short data_length;
   char data[PACKET_VALUE_MAX_SIZE];
};

#ifdef SENSACT_CLIENT
extern unsigned short packet_counter;

int create_request_packet(struct request_packet_t *packet, int command, char *name, void *set_value, int set_value_length, unsigned short *id);
int verify_response_packet(struct response_packet_t *packet, unsigned short id);
#endif // SENSACT_CLIENT

#ifdef SENSACT_SERVER
int verify_request_packet(struct request_packet_t *packet);
int decode_request_packet(struct request_packet_t *packet, int *command, char *name, char *value, unsigned short *id);
int create_response_packet(struct response_packet_t *packet, int command, char *data, int length, unsigned short id);
#endif // SENSACT_SERVER

#endif
