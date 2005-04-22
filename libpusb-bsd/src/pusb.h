/*
*  ALCATEL SpeedTouch USB : Portable USB user library
*  Copyright (C) 2001 Benoit PAPILLAULT
*  
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*  
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  Author   : Benoit PAPILLAULT <benoit.papillault@free.fr>
*  Creation : 29/05/2001
*
* $Id$
*/

#ifndef PUSB_H
#define PUSB_H

/* Simple portable USB interface */

typedef struct pusb_enum     * pusb_enum_t;
typedef struct pusb_device   * pusb_device_t;
typedef struct pusb_endpoint * pusb_endpoint_t;

int  pusb_enum_init(pusb_enum_t * ref);
int  pusb_enum_next(pusb_enum_t * ref);
void pusb_enum_done(pusb_enum_t * ref);

int pusb_get_device_class(pusb_enum_t * ref);
int pusb_get_device_subclass(pusb_enum_t * ref);
int pusb_get_device_protocol(pusb_enum_t * ref);

int pusb_get_device_vendor(pusb_enum_t * ref);
int pusb_get_device_product(pusb_enum_t * ref);
int pusb_get_device_version(pusb_enum_t * ref);

pusb_device_t pusb_device_open(pusb_enum_t * ref);
int pusb_device_close(pusb_device_t dev);

int pusb_device_set_configuration(pusb_device_t dev, int config);
int pusb_device_set_interface(pusb_device_t dev, int interface, int alternate);

/*
  pusb_endpoint_open: epnum is 8 bits and indicates both the endpoint number
  and the read/write flag (bit 7). In this way, 0x02 and 0x82 are considered
  different endpoints!

  According to USB 2.0 Specifications, epnum is only 4 bits, plus the
  bit 7 for the direction : 0 = OUT, 1 = IN.
*/

pusb_endpoint_t pusb_endpoint_open(pusb_device_t dev, int epnum);
int pusb_endpoint_close(pusb_endpoint_t ep);

int pusb_control_msg(pusb_device_t dev,
                     int request_type, int request,
                     int value, int index,
                     unsigned char *buf, int size);

int pusb_endpoint_bulk_read(pusb_endpoint_t ep,
                            unsigned char *buf, int size);
int pusb_endpoint_bulk_write(pusb_endpoint_t ep,
                             const unsigned char *buf, int size);

int pusb_endpoint_int_read(pusb_endpoint_t ep,
                           unsigned char *buf, int size);
int pusb_endpoint_int_write(pusb_endpoint_t ep,
                            const unsigned char *buf, int size);

int pusb_endpoint_iso_read(pusb_endpoint_t ep,
                           unsigned char *buf, int size);
int pusb_endpoint_iso_write(pusb_endpoint_t ep,
                            const unsigned char *buf, int size);

/* common function */

pusb_device_t pusb_search_open(int vendorID, int productID);

#endif
