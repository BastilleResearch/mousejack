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


#include "usb_desc.h"

// Device descriptor 
__code const device_descriptor_t device_descriptor = 
{
  .bLength            = 18,     // Size of this struct
  .bDescriptorType    = DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200, // USB 2.0
  .bDeviceClass       = 0xFF,
  .bDeviceSubClass    = 0xFF,
  .bDeviceProtocol    = 0xFF,
  .bMaxPacketSize0    = 64,     // EP0 max packet size
  .idVendor           = 0x1915, // Nordic Semiconductor
  .idProduct          = 0x0102, // Nordic bootloader product ID incremebted by 1
  .bcdDevice          = 0x0001, // Device version number 
  .iManufacturer      = STRING_DESCRIPTOR_MANUFACTURER,
  .iProduct           = STRING_DESCRIPTOR_PRODUCT,
  .iSerialNumber      = 0,
  .bNumConfigurations = 1,      // Configuration count
};

// Configuration descriptor 
__code const configuration_descriptor_t configuration_descriptor = 
{
  .bLength                = 9,     // Size of the configuration descriptor
  .bDescriptorType        = CONFIGURATION_DESCRIPTOR,
  .wTotalLength           = 32,    // Total size of the configuration descriptor and EP/interface descriptors 
  .bNumInterfaces         = 1,     // Interface count
  .bConfigurationValue    = 1,     // Configuration identifer
  .iConfiguration         = 0,
  .bmAttributes           = 0x80,  // Bus powered
  .bMaxPower              = 100,   // Max power of 100*2mA = 200mA 
  .interface_descriptor = 
    {
      .bLength            = 9,    // Size of the interface descriptor 
      .bDescriptorType    = INTERFACE_DESCRIPTOR,
      .bInterfaceNumber   = 0,    // Interface index
      .bAlternateSetting  = 0,   
      .bNumEndpoints      = 2,    // 2 endpoints, EP1IN, EP1OUT
      .bInterfaceClass    = 0xFF, // Vendor interface class
      .bInterfaceSubClass = 0xFF, // Vendor interface subclass
      .bInterfaceProtocol = 0xFF,
      .iInterface         = 0,
    },
  .endpoint_1_in_descriptor = 
    {
      .bLength            = 7,    // Size of the endpoint descriptor 
      .bDescriptorType    = ENDPOINT_DESCRIPTOR,
      .bEndpointAddress   = 0x81, // EP1 IN
      .bmAttributes       = 0x02, // Bulk EP
      .wMaxPacketSize     = 64,   // 64 byte packet buffer
      .bInterval          = 0, 
    },
  .endpoint_1_out_descriptor = 
    {
      .bLength            = 7,    // Size of the endpoint descriptor 
      .bDescriptorType    = ENDPOINT_DESCRIPTOR,
      .bEndpointAddress   = 0x01, // EP1 OUT
      .bmAttributes       = 0x02, // Bulk EP
      .wMaxPacketSize     = 64,   // 64 byte packet buffer
      .bInterval          = 0,
    },
};

// String descriptor values 
__code char * device_strings[3] = 
{
  "\x04\x09",          // Language (EN-US)
  "RFStorm",           // Manufacturer
  "Research Firmware", // Product
};

