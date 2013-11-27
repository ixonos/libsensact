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
#include <libusb.h>
#include "usb.h"
#include "session.h"
#include "debug.h"

int usb_connect(struct libusb_device_handle **usb_device, int vid, int pid, int endpoint)
{
   libusb_device_handle *device;

   libusb_init(NULL);

   device = libusb_open_device_with_vid_pid(NULL, vid, pid);

   if (device != NULL)
   {
      libusb_detach_kernel_driver(device, 0);

      int claimed = libusb_claim_interface(device, 0);

      if (claimed == 0)
      {
         debug_printf("Connected to USB device (vid=%X,pid=%X)\n", vid, pid);

         // Store USB device handle for session
         *usb_device = device;

         return 0;
      }

      libusb_close(device);
   }

   printf("Error: Could not connect USB device (vid=%X,pid=%X)\n", vid, pid);

   return -1;
}

int usb_disconnect(int handle)
{
   debug_printf("Disconnecting USB device...\n");

   // Release USB interface
   libusb_release_interface(session[handle].usb.device, 0);
   libusb_close(session[handle].usb.device);

   return 0;
}

int usb_reconnect(int handle)
{
   int ret;
   ret = usb_disconnect(handle);
   if (ret != 0)
     return ret;

   ret = usb_connect(&session[handle].usb.device, session[handle].device->vid,
       session[handle].device->pid, session[handle].device->endpoint);

   return ret;
}

int usb_write(int handle, char *data, int length, int timeout)
{
   int i;
   int count;
   int ret;

   // Debug
   debug_printf("Sending data,  %d bytes (USB): ", length);
   for (i=0; i<length; i++)
      debug_printf_raw("0x%x ", (unsigned char) data[i]);
   debug_printf_raw("\n");

   // Write bulk message
  ret = libusb_bulk_transfer(session[handle].usb.device,
                             session[handle].device->endpoint | LIBUSB_ENDPOINT_OUT,
                             (unsigned char *)data, length,
                             &count, timeout);

  if (ret == 0)
     return count;
  else
  {
     printf("Error: USB write failed (%d) %s\n", ret, libusb_error_name(ret));
     return ret;
  }
}

int usb_read(int handle, char *data, int length, int timeout)
{
   int count;
   int ret;
   int i;

   // Read bulk message
   ret = libusb_bulk_transfer(session[handle].usb.device,
                          session[handle].device->endpoint | LIBUSB_ENDPOINT_IN,
                          (unsigned char *) data, length,
                          &count, timeout);

   // Debug
   debug_printf("Received data, %d bytes (USB): ", count);
   for (i=0; i<count; i++)
      debug_printf_raw("0x%x ", (unsigned char) data[i]);
   debug_printf_raw("\n");

   if (ret == 0)
      return count;
   else
   {
     printf("Error: USB read failed (%d) %s\n", ret, libusb_error_name(ret));
     return ret;
   }
}
