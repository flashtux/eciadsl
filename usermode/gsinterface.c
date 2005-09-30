/*
  Author : kolja <gava@bergamoblog.it>
  Creation : 29/11/2003
  Licence : GPL

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************
  GlobeSpan Chipset Common Interface
     cantain all the information related to device 
     this interface will load infos and methods from the right chipset depending on
     compile option (see ./configure --help | --with-chipset option)
*/

#include <string.h>
#include "pusb.h"
#include "gsinterface.h"

struct eci_device_t eci_device;

#include "gs7470.c"
#include "gs7070.c"

#define TIMEOUT 2000

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80


/* from table 9.4 in USB Specs Rev 1.1 => http://www.usb.org/ */

enum
{
	USB_REQ_GET_STATUS        = 0,
	USB_REQ_CLEAR_FEATURE     = 1,
	USB_REQ_SET_FEATURE       = 3,
	USB_REQ_SET_ADDRESS       = 5,
	USB_REQ_GET_DESCRIPTOR    = 6,
	USB_REQ_SET_DESCRIPTOR    = 7,
	USB_REQ_GET_CONFIGURATION = 8,
	USB_REQ_SET_CONFIGURATION = 9,
	USB_REQ_GET_INTERFACE     = 10,
	USB_REQ_SET_INTERFACE     = 11,
	USB_REG_SYNCH_FRAME       = 12
};

/* from table 9.5 in USB Specs Rev 1.1 => http://www.usb.org/ */

enum
{
	USB_DESCRIPTOR_DEVICE        = 1,
	USB_DESCRIPTOR_CONFIGURATION = 2,
	USB_DESCRIPTOR_STRING        = 3,
	USB_DESCRIPTOR_INTERFACE     = 4,
	USB_DESCRIPTOR_ENDPOINT      = 5
};

/* from $9.6.1 in USB Specs Rev 1.1 => http://www.usb.org/ */

typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
} __attribute__ ((packed)) usb_desc_header_t;

typedef struct 
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short bcdUSB;
	unsigned char bDeviceClass;
	unsigned char bDeviceSubClass;
	unsigned char bDeviceProtocol;
	unsigned char bMaxPacketSize0;
	unsigned short idVendor;
	unsigned short idProduct;
	unsigned short bcdDevice;
	unsigned char iManufacturer;
	unsigned char iProduct;
	unsigned char iSerialNumber;
	unsigned char bNumConfigurations;
} __attribute__ ((packed)) usb_desc_device_t;

typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short wTotalLength;
	unsigned char bNumInterfaces;
	unsigned char bConfigurationValue;
	unsigned char iConfiguration;
	unsigned char bmAttributes;
	unsigned char MaxPowser;
} __attribute__ ((packed)) usb_desc_configuration_t;

typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bInterfaceNumber;
	unsigned char bAlternateSetting;
	unsigned char bNumEndpoints;
	unsigned char bInterfaceClass;
	unsigned char bInterfaceSubClass;
	unsigned char bInterfaceProtocol;
	unsigned char iInterface;
} __attribute__ ((packed)) usb_desc_interface_t;

typedef struct
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
	unsigned short wMaxPacketSize;
	unsigned char bInterval;
} __attribute__ ((packed)) usb_desc_endpoint_t;

void (*pF_getAal5HeaderStructure)(const unsigned char*,
                                  struct aal5_header_st*);

/* set eci modem chipset - kolja */
void set_eci_modem_chipset(char* chipset){
	if (memcmp(chipset, "GS7470", 6)==0){
		gs7470InitParams();
		pF_getAal5HeaderStructure = getAal5HeaderStructure7470;				
	}else{
		gs7070InitParams();
		pF_getAal5HeaderStructure = getAal5HeaderStructure7070;			
	}
}

/* set eci modem devie altIface Descriptor infos - kolja 
 *  (retrieves informations related to ep handled by required altIface
 *   and set it up on eci_device structure) */
inline int gsGetDeviceIfaceInfo(pusb_device_t dev, unsigned short int alt_interface){
	usb_desc_device_t device;
	/* we fixe to use Interface 0  - kolja*/
	int conf = 0;
	usb_desc_configuration_t configuration;
	unsigned char * mem;
	int len, idx;
	unsigned int altiface_found=0;
	usb_desc_endpoint_t *ep_desc;
	unsigned int ep_count=0;
	usb_desc_header_t * hdr;

	if (pusb_control_msg(dev, 0x80, USB_REQ_GET_DESCRIPTOR,
						 (USB_DESCRIPTOR_DEVICE << 8), 0,
						 (unsigned char *)&device, sizeof(device), TIMEOUT) <0)
	{
		perror("pusb_control_msg");
		return -1;
	}
          
	if (pusb_control_msg(dev, 0x80, USB_REQ_GET_DESCRIPTOR,
						 (USB_DESCRIPTOR_CONFIGURATION << 8) | conf, 0,
						 (unsigned char *)&configuration,
						 sizeof(configuration), TIMEOUT) <0)
	{
		perror("pusb_control_msg");
		return -1;
	}

	/* warning : the following line only works on Little Endian Arch. */
	len = configuration.wTotalLength;
	mem = (unsigned char *)malloc(len);
	if (mem == NULL)
	{
		perror("malloc");
		return -1;
	}
	
	if (pusb_control_msg(dev, 0x80, USB_REQ_GET_DESCRIPTOR,
						 (USB_DESCRIPTOR_CONFIGURATION << 8) | conf, 0,
						 mem, len, TIMEOUT) <0)
	{
		perror("pusb_control_msg");
		return -1;
	}

	for (idx=0;idx<len;)
	{
		hdr = (usb_desc_header_t *)(mem+idx);

		switch (hdr->bDescriptorType)
		{
/*		case USB_DESCRIPTOR_DEVICE:
			break;
		case USB_DESCRIPTOR_CONFIGURATION:
			break;
		case USB_DESCRIPTOR_STRING:
			break;
*/
		case USB_DESCRIPTOR_INTERFACE:
			altiface_found=0;
			if ((unsigned int)((usb_desc_interface_t *)hdr)->bAlternateSetting == alt_interface){
				altiface_found=1;
			}
			break;	
		case USB_DESCRIPTOR_ENDPOINT:
			if (altiface_found){
				ep_count++;
				ep_desc = (usb_desc_endpoint_t *)hdr;
				switch(ep_count){
					case 1:
						switch(ep_desc->bmAttributes){
							case USB_ENDPOINT_XFER_BULK:
								eci_device.eci_in_ep = ep_desc->bEndpointAddress;
								eci_device.use_datain_iso_urb=0;
								break;
							case USB_ENDPOINT_XFER_ISOC:
								eci_device.eci_in_ep = ep_desc->bEndpointAddress;
								eci_device.use_datain_iso_urb=1;
								eci_device.eci_iso_packet_size = (short int)ep_desc->wMaxPacketSize;
								break;
						}
						break;
					case 2:
						eci_device.eci_out_ep = ep_desc->bEndpointAddress;
						break;
					case 3:
						eci_device.eci_int_ep = ep_desc->bEndpointAddress;
						break;
				}
			}
			break;
		}
		idx += hdr->bLength;
	}
	return 0;
}

const char * get_chipset_descr(eci_device_chiset eci_chipset){
	if (eci_chipset == ECIDC_GS7470)
		return("GS7470");
	else
		return("GS7070");
}

inline void getAal5HeaderStructure(const unsigned char* aal5Header,
                            struct aal5_header_st* aal5HeaderOut)
{
	pF_getAal5HeaderStructure(aal5Header, aal5HeaderOut);
}
