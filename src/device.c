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
#include "device.h"

/* List of supported sensor/actuator devices */
struct device_t device[] =
{
   {  .name = "senshub0",
      .description = "TI Tiva Launchpad device (with Sensehub)",
      .connection = USB,
      .vid = 0x1CBE,
      .pid = 0x0003,
      .endpoint = 0x1 },

   {  .name = "dummy0",
      .description = "TI Tiva Launchpad device (dummy)",
      .connection = USB,
      .vid = 0x1CBE,
      .pid = 0x0040,
      .endpoint = 0x1 },

   {  .name = "nfc0",
      .description = "TI Tiva Launchpad device (with NFC)",
      .connection = USB,
      .vid = 0x1CBE,
      .pid = 0x0041,
      .endpoint = 0x1 },

   {  .name = "motor0",
      .description = "TI Tiva Launchpad device (with motor)",
      .connection = USB,
      .vid = 0x1CBE,
      .pid = 0x0042,
      .endpoint = 0x1 },

   {  .name = "led_array0",
      .description = "TI Tiva Launchpad device (with LED array)",
      .connection = USB,
      .vid = 0x1CBE,
      .pid = 0x0043,
      .endpoint = 0x1 },

   { }
};

