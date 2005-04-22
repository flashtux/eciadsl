/*
  Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 01/07/2001

  07/10/2003 : ported to libpusb.
*/

#include <stdio.h>
#include <stdlib.h>

#include "pusb.h"

#define TIMEOUT 100

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

void print_usb_desc_string(usb_desc_header_t *p)
{
	int i;
	unsigned char * s = (unsigned char *)p;

	if (p->bDescriptorType != USB_DESCRIPTOR_STRING)
		printf("bDescriptorType = %u instead of %u\n",
			   p->bDescriptorType,USB_DESCRIPTOR_STRING);
/*
	for (i=0;i<p->bLength;i++)
		printf("%02x ",s[i]);
*/
	for (i=2;i<p->bLength;i+=2)
		printf("%c",s[i]);
}

void print_string(pusb_device_t fdusb, int iString)
{
	unsigned char buf[1000];

	if (iString == 0)
		return ;

	if (pusb_control_msg(fdusb,0x80,USB_REQ_GET_DESCRIPTOR,
						 (USB_DESCRIPTOR_STRING << 8) | iString,0,
						 buf,sizeof(buf)) <0)
	{
		perror("pusb_control_msg");
		return;
	}

	print_usb_desc_string((usb_desc_header_t *)buf);
}

void print_usb_desc_device(pusb_device_t fdusb,usb_desc_device_t * p)
{
/*
	printf(" === DEVICE ===\n");
	printf("bLength = %u\n",p->bLength);
	printf("bDescriptorType = %u\n",p->bDescriptorType);
	printf("idVendor = 0x%x\n",p->idVendor);
	printf("idProduct = 0x%x\n",p->idProduct);
	printf("bNumConfigurations = %u\n",p->bNumConfigurations);
*/
	if (p->bLength != sizeof(*p))
		printf("bLength = %u instead of %u\n",p->bLength,sizeof(*p));
	if (p->bDescriptorType != USB_DESCRIPTOR_DEVICE)
		printf("bDescriptorType = %u instead of %u\n",
			   p->bDescriptorType,USB_DESCRIPTOR_DEVICE);
	printf("Device: VendorID=0x%04x ProductID=0x%04x Class=%02x/%02x/%02x, %u configuration(s)\n",
		   p->idVendor,p->idProduct,
		   p->bDeviceClass,p->bDeviceSubClass,p->bDeviceProtocol,
		   p->bNumConfigurations);
	printf("Manufacturer: ");
	print_string(fdusb,p->iManufacturer);
	printf(" Product: ");
	print_string(fdusb,p->iProduct);
	printf(" SN: ");
	print_string(fdusb,p->iSerialNumber);
	printf("\n");
}

void print_usb_desc_configuration(pusb_device_t fdusb,
								  usb_desc_configuration_t *p)
{
/*
	printf(" === CONFIGURATION ===\n");
	printf("bLength = %u\n",p->bLength);
	printf("bDescriptorType = %u\n",p->bDescriptorType);
	printf("wTotalLength = %u\n",p->wTotalLength);
	printf("bNumInterfaces = %u\n",p->bNumInterfaces);
	printf("bConfigurationValue = %u\n",p->bConfigurationValue);
	printf("iConfiguration = %u\n",p->iConfiguration);
	printf("bmAttributes = 0x%x\n",p->bmAttributes);
	printf("MaxPowser = %u mA\n",p->MaxPowser * 2);
*/
	if (p->bLength != sizeof(*p))
		printf("bLength = %u instead of %u\n",p->bLength,sizeof(*p));
	if (p->bDescriptorType != USB_DESCRIPTOR_CONFIGURATION)
		printf("bDescriptorType = %u instead of %u\n",
			   p->bDescriptorType,USB_DESCRIPTOR_CONFIGURATION);
	printf("  configuration %u, %u interface(s) [",
		   p->bConfigurationValue,p->bNumInterfaces);
	print_string(fdusb,p->iConfiguration);
	printf("]\n");
}

void print_usb_desc_interface(pusb_device_t fdusb,usb_desc_interface_t *p)
{
/*
	printf(" === INTERFACE ===\n");
	printf("bLength = %u\n",p->bLength);
	printf("bDescriptorType = %u\n",p->bDescriptorType);
	printf("bInterfaceNumber = %u\n",p->bInterfaceNumber);
	printf("bAlternateSetting = %u\n",p->bAlternateSetting);
	printf("bNumEndpoints = %u\n",p->bNumEndpoints);
	printf("bInterfaceClass = %u\n",p->bInterfaceClass);
	printf("bInterfaceSubClass = %u\n",p->bInterfaceSubClass);
	printf("bInterfaceProtocol = %u\n",p->bInterfaceProtocol);
	printf("iInterface = %u\n",p->iInterface);
*/
	if (p->bLength != sizeof(*p))
		printf("bLength = %u instead of %u\n",p->bLength,sizeof(*p));
	if (p->bDescriptorType != USB_DESCRIPTOR_INTERFACE)
		printf("bDescriptorType = %u instead of %u\n",
			   p->bDescriptorType,USB_DESCRIPTOR_INTERFACE);
	printf("    interface %u alt %u class %02x/%02x/%02x, %u endpoint(s) [",
		   p->bInterfaceNumber,p->bAlternateSetting,
		   p->bInterfaceClass,p->bInterfaceSubClass,p->bInterfaceProtocol,
		   p->bNumEndpoints);
	print_string(fdusb,p->iInterface);
	printf("]\n");
}

const char * endpoint_type[] =
{
	"Ctrl", "Isoc", "Bulk", "Intr"
};

void print_usb_desc_endpoint(usb_desc_endpoint_t *p)
{
/*
	printf(" === ENDPOINT ===\n");
	printf("bLength = %u\n",p->bLength);
	printf("bDescriptorType = %u\n",p->bDescriptorType);
	printf("bEndpointAddress = 0x%x\n",p->bEndpointAddress);
	printf("bmAttributes = 0x%x\n",p->bmAttributes);
	printf("wMaxPacketSize = %u bytes\n",p->wMaxPacketSize);
	printf("bInterval = %u ms\n",p->bInterval);
*/
	if (p->bLength != sizeof(*p))
		printf("bLength = %u instead of %u\n",p->bLength,sizeof(*p));
	if (p->bDescriptorType != USB_DESCRIPTOR_ENDPOINT)
		printf("bDescriptorType = %u instead of %u\n",
			   p->bDescriptorType,USB_DESCRIPTOR_ENDPOINT);
	printf("      endpoint 0x%02x [%s] %u bytes %u ms\n",
		   p->bEndpointAddress,endpoint_type[p->bmAttributes&3],
		   p->wMaxPacketSize,p->bInterval);
}

void usage()
{
	printf("usage: pusb-view vendorID productID\n");
	exit (-1);
}

int display_device(pusb_device_t fdusb)
{
	usb_desc_device_t device;
	int conf;

	if (pusb_control_msg(fdusb,0x80,USB_REQ_GET_DESCRIPTOR,
						 (USB_DESCRIPTOR_DEVICE << 8),0,
						 (unsigned char *)&device,sizeof(device)) <0)
	{
		perror("pusb_control_msg");
		return -1;
	}
    
	print_usb_desc_device(fdusb,&device);
    
	for (conf=0;conf<device.bNumConfigurations;conf++)
	{
		usb_desc_configuration_t configuration;
		unsigned char * mem;
		int len, idx;
        
		if (pusb_control_msg(fdusb,0x80,USB_REQ_GET_DESCRIPTOR,
							 (USB_DESCRIPTOR_CONFIGURATION << 8) | conf,0,
							 (unsigned char *)&configuration,
							 sizeof(configuration)) <0)
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
		
		if (pusb_control_msg(fdusb,0x80,USB_REQ_GET_DESCRIPTOR,
							 (USB_DESCRIPTOR_CONFIGURATION << 8) | conf,0,
							 mem,len) <0)
		{
			perror("pusb_control_msg");
			return -1;
		}

		for (idx=0;idx<len;)
		{
			usb_desc_header_t * hdr = (usb_desc_header_t *)(mem+idx);

			switch (hdr->bDescriptorType)
			{
			case USB_DESCRIPTOR_DEVICE:
				print_usb_desc_device(fdusb,(usb_desc_device_t *)hdr);
				break;
			case USB_DESCRIPTOR_CONFIGURATION:
				print_usb_desc_configuration(fdusb,
											 (usb_desc_configuration_t *)hdr);
				break;
			case USB_DESCRIPTOR_STRING:
				break;
			case USB_DESCRIPTOR_INTERFACE:
				print_usb_desc_interface(fdusb,(usb_desc_interface_t *)hdr);
				break;
			case USB_DESCRIPTOR_ENDPOINT:
				print_usb_desc_endpoint((usb_desc_endpoint_t *)hdr);
				break;
			}

			idx += hdr->bLength;
		}
	}
}

int main(int argc,char *argv[])
{
    pusb_enum_t ref;
	pusb_device_t fdusb;
    
    if (!pusb_enum_init(&ref))
    {
        fprintf(stderr,"cannot enumerate USB bus\n");
        return -1;
    }

    while (pusb_enum_next(&ref))
    {
/*
        printf("  Class: %02x SubClass: %02x Protocol: %02x\n",
               pusb_get_device_class(&ref),
               pusb_get_device_subclass(&ref),
               pusb_get_device_protocol(&ref));
        printf("  Vendor: %04x Product: %04x Version: %x.%x\n",
               pusb_get_device_vendor(&ref),
               pusb_get_device_product(&ref),
               pusb_get_device_version(&ref) >> 8,
               pusb_get_device_version(&ref) & 0xff);
*/      
        fdusb = pusb_device_open(&ref);
        display_device(fdusb);
        pusb_device_close(fdusb);
    }

    pusb_enum_done(&ref);

    return 0;
} 
