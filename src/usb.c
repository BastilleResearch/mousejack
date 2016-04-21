/*
  Copyright (C) 2016 Bastille Networks

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "usb.h"

// xdata mapped USB request setup buffer
__xdata struct usb_request_t * request = (__xdata void*)setupbuf;

// Initialize the USB configuraiton 
bool init_usb() 
{
  uint16_t ms_elapsed = 0; 
  configured = false;

  // Wakeup USB
  usbcon = 0x40; 

  // Reset the USB bus
  usbcs |= 0x08;
  delay_us(50000);
  usbcs &= ~0x08;

  // Set the default configuration 
  usb_reset_config();

  // Wait for the USB controller to reach the configured state
  while(!configured);

  // Device configured successfully
  return true;
}

// Reset the USB configuration
void usb_reset_config()
{
  // Setup interrupts 
  usbien = 0x11;  // USB reset and setup data valid
  in_ien = 0x00;  // Disable EP IN interrupts
  out_ien = 0x02; // Enable EP1 OUT interrupt
  ien1 = 0x10;    // Enable USB interrupt
  in_irq = 0x1F;  // Clear IN IRQ flags
  out_irq = 0x1F; // Clear OUT IRQ flags
  
  // Enable bulk EP1, disable ISO EPs
  inbulkval = 0x02;
  outbulkval = 0x02;
  inisoval = 0x00;
  outisoval = 0x00;  

  // Setup EP buffers
  bout1addr = 32;
  bout2addr = 64;
  binstaddr = 16;
  bin1addr  = 32;
  bin2addr  = 64;
  out1bc    = 0xFF;
}

// USB IRQ handler
void usb_irq() __interrupt(12)  __using(1)
{
  // Which IRQ? 
  // ref: nRF24LU1+ Product Spec, Section 7.8.3, Table 34
  switch (ivec) 
  {
    // Setup data available 
    case 0x00:
      handle_setup_request();
      usbirq = 0x01;
      break;

    // Reset to initial state
    case 0x10:
      usb_reset_config();
      usbirq = 0x10;
      break;

    // EP1 out (request from host)
    case 0x24:
      handle_radio_request(out1buf[0], &out1buf[1]);
      out_irq = 0x02;
      out1bc = 0xFF;
      break;
  }
}

// Convert a device string to unicode, and write it to EP0
void write_device_string(const char * string)
{
  int x;
  int length = strlen(string);
  memset(in0buf+2, 0, 64);
  in0buf[0] = 2+length*2;
  in0buf[1] = STRING_DESCRIPTOR;
  for(x = 0; x < length; x++) in0buf[2+x*2] = string[x];
  in0bc = in0buf[0];
}

// Write a descriptor (as specified in the current request) to EP0
bool write_descriptor()
{
  uint8_t desc_len = request->wLength;

  switch(request->wValue >> 8)
  {
    // Device descriptor request
    case DEVICE_DESCRIPTOR:
      if(desc_len > device_descriptor.bLength) desc_len = device_descriptor.bLength;
      memcpy(in0buf, &device_descriptor, desc_len);
      in0bc = desc_len;
      return true;
  
    // Configuration descriptor request
    case CONFIGURATION_DESCRIPTOR:
      if(desc_len > configuration_descriptor.wTotalLength) desc_len = configuration_descriptor.wTotalLength;
      memcpy(in0buf, &configuration_descriptor, desc_len);
      in0bc = desc_len;
      return true;

    // String descriptor request
    // - Language, Manufacturer, or Product
    case STRING_DESCRIPTOR:
       write_device_string(device_strings[setupbuf[2]]);
       return true;   
  }  

  // Not handled
  return false;
}

// Handle a USB setup request 
void handle_setup_request()
{
  bool handled = false;
  switch(request->bRequest)
  {
    // Return a descriptor 
    case GET_DESCRIPTOR:
      if(write_descriptor()) handled = true;
      break;

    // The host has assigned an address, but we don't have to do anything
    case SET_ADDRESS:
      handled = true;
      break;

    // Set the configuration state
    case SET_CONFIGURATION:   
      if (request->wValue == 0) configured = false; // Not configured, drop back to powered state
      else configured = true;                       // Configured
      handled = true;
      break;

    // Get the configuration state
    case GET_CONFIGURATION:
      in0buf[0] = configured;
      in0bc = 1;
      handled = true;
      break;

    // Get endpoint or interface status
    case GET_STATUS:

      // Endpoint status
      if (request->bmRequestType == 0x82)
      {
        if ((setupbuf[4] & 0x80) == 0x80) in0buf[0] = in1cs;
        else in0buf[0] = out1cs; 
      }

      // Device / Interface status, always two 0 bytes because 
      // we're bus powered and don't support remote wakeup 
      else 
      {
        in0buf[0] = 0;
        in0buf[1] = 0;
      }
      
      in0bc = 2;
      handled = true;
      break;
  }

  // Stall if the request wasn't handled
  if(handled) ep0cs = 0x02; // hsnak
  else ep0cs = 0x01; // ep0stall
}
