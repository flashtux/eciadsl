/*
  Author   : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 29/05/2001

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
 Licence	:	GPL
*********************************************************************

  Portable USB user library -- Linux implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <signal.h>
#include <errno.h>
#include <string.h>

#include <linux/limits.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <asm/page.h>

struct pusb_endpoint_t
{
	int fd;
	int ep;
};

#include "pusb.h"

struct pusb_device_t
{
	int fd;
};


#define MAXBUFFER 20

struct pusb_urb_t
{
	int ep;
	int buf_nb;
	unsigned char* buf [MAXBUFFER];
	int buf_size[MAXBUFFER];
};

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

static const char usb_path[] = "/proc/bus/usb";

/* Device descriptor */
struct usb_device_descriptor
{
	__u8  bLength;
	__u8  bDescriptorType;
	__u16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__u16 idVendor;
	__u16 idProduct;
	__u16 bcdDevice;
	__u8  iManufacturer;
	__u8  iProduct;
	__u8  iSerialNumber;
	__u8  bNumConfigurations;
} __attribute__ ((packed));

/* try to open the file and get USB device information,
   if it's ok, check if it matches vendorID & productID and return
   a usable file descriptor in this case. Else returns -1.
*/

int test_file(const char* path, int vendorID, int productID)
{
	int fd;
	struct usb_device_descriptor desc;
	
	fd = open(path, O_RDWR);
	if (fd == -1)
    {
		perror(path);
		return -1;
    }
  
	if (read(fd, &desc, sizeof(desc)) == sizeof(desc))
    {
		/* great, we read something */
		
		/* check, it match the correct structure */

		if (desc.bLength == sizeof(desc))
		{
			/*	  
				  printf("=== %s ===\n", path);
				  printf("  bLength         = %u\n", desc.bLength);
				  printf("  bDescriptorType = %u\n", desc.bDescriptorType);
				  printf("  bcdUSB          = %04x\n", desc.bcdUSB);
				  printf("  idVendor        = %04x\n", desc.idVendor);
				  printf("  idProduct       = %04x\n", desc.idProduct);
				  printf("  bcdDevice       = %04x\n", desc.bcdDevice);
			*/
			if (vendorID == desc.idVendor
				&& productID == desc.idProduct)
				return(fd);
		}
    }
	
	close(fd);
	return(-1);
}

/* search for a vendorID, productID. return -1 if search failed
   or a usable file descriptor
*/

int usbfs_search(const char* path, int vendorID, int productID)
{
	int result = -1;
	
	DIR* dir;
	struct dirent* dirp;
	
	dir = opendir(path);
	if (dir == NULL)
    {
		perror(path);
		return(-1);
    }
	
	while ((dirp=readdir(dir)) != NULL)
    {
		char file[PATH_MAX+1];
		struct stat statbuf;
		
		if (strcmp(dirp->d_name, ".")==0)
			continue;
		if (strcmp(dirp->d_name, "..")==0)
			continue;
		
		sprintf(file, "%s/%s", path, dirp->d_name);
		
		if (stat(file, &statbuf)!=0)
		{
			perror(file);
			continue;
		}
		
		if (S_ISDIR(statbuf.st_mode))
		{
			if ((result = usbfs_search(file, vendorID, productID)) < 0)
				continue;
			else
				break;
		}
		
		if (S_ISREG(statbuf.st_mode))
		{
			/* check the file size, which must be 18 
			   = sizeof(struct usb_device_descriptor) */
			
			if (statbuf.st_size != sizeof(struct usb_device_descriptor))
				continue;
			
			if ((result=test_file(file, vendorID, productID)) < 0)
				continue;
			else
				break;
		}
    }
	
	closedir(dir);
	return(result);
}

static pusb_device_t make_device(int fd)
{
    pusb_device_t dev;

    if (!(dev = malloc(sizeof(*dev))))
	{
		close (fd);
		return(0);
	}
	
    dev->fd = fd;
    return(dev);
}

pusb_device_t pusb_search_open(int vendorID, int productID)
{
	int fd;

	fd = usbfs_search("/proc/bus/usb", vendorID, productID);
	if (fd < 0)
		return(NULL);

	return(make_device(fd));
}

pusb_device_t pusb_open(const char* path)
{
	int fd; 

	fd = open(path, O_RDWR);
	if (fd < 0)
	{
		perror(path);
		return(NULL);
	}

	return(make_device(fd));
}

int pusb_close(pusb_device_t dev)
{
	int ret;

	ret = close (dev->fd);
	free(dev);

	return(ret);
}

int pusb_control_msg(pusb_device_t dev,
		     int request_type, int request,
		     int value, int index, 
		     unsigned char* buf, int size, int timeout)
{
	int ret;
	struct usbdevfs_ctrltransfer ctrl;
/*
	printf("URB xxx: out request_type=0x%02x request=%08x value=%08x index=%08x\n",
		   request_type, request, value, index);
	dump(buf, size);
*/
	ctrl.requesttype = request_type;
	ctrl.request     = request;
	ctrl.value       = value;
	ctrl.index       = index;
	ctrl.length      = size;
	ctrl.timeout     = timeout;
	ctrl.data        = buf;

	ret = ioctl(dev->fd, USBDEVFS_CONTROL, &ctrl);
	return(ret);
}

int pusb_set_configuration(pusb_device_t dev, int config)
{
  int ret;

  ret = ioctl(dev->fd, USBDEVFS_SETCONFIGURATION, &config);
  return(ret);
}

int pusb_set_interface(pusb_device_t dev, int interface, int alternate)
{
  struct usbdevfs_setinterface setintf;
  int ret;

  setintf.interface = interface;
  setintf.altsetting = alternate;

  ret = ioctl(dev->fd, USBDEVFS_SETINTERFACE, &setintf);
  return(ret);
}

pusb_endpoint_t pusb_endpoint_open(pusb_device_t dev, int epnum, int flags)
{
	pusb_endpoint_t ep;
	int foo;

	foo=flags||foo; /* trick for pedantic C compilers */
	ep = (pusb_endpoint_t)malloc(sizeof(*ep));
	if (ep == NULL)
		return(NULL);

	ep->fd = dev->fd;
	ep->ep = epnum & 0xf;

	return(ep);
}

int pusb_endpoint_rw_no_timeout(int fd, int ep,
		       unsigned char* buf, int size)
{
	struct usbdevfs_urb urb, *purb = &urb;
	int ret;

	memset(purb, 0, sizeof(urb));

	purb->type = USBDEVFS_URB_TYPE_BULK;
	purb->endpoint = ep;
	purb->flags  = 0;
	purb->buffer = buf;
	purb->buffer_length = size;
	purb->signr = 0;

	do
	{
		ret = ioctl(fd, USBDEVFS_SUBMITURB, purb);
	}
	while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return(ret);

	do
	{
		ret = ioctl(fd, USBDEVFS_REAPURB, &purb);
	}
	while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return(ret);

	if (purb != &urb)
		printf("purb=%p, &urb=%p\n", (void*)purb, (void*)&urb);

	if (purb->buffer != buf)
		printf("purb->buffer=%p, buf=%p\n", (void*)purb->buffer, (void*)buf);

	return(purb->actual_length);
}

int pusb_endpoint_rw(int fd, int ep, unsigned char* buf, int size, int timeout)
{
	struct usbdevfs_bulktransfer bulk;
	int ret, received = 0;

	do
    {
		bulk.ep      = ep;
		bulk.len     = size;
		if (bulk.len > PAGE_SIZE)
			bulk.len = PAGE_SIZE;
		bulk.timeout = timeout;
		bulk.data    = buf;

		do
		{
			ret = ioctl(fd, USBDEVFS_BULK, &bulk);
		}
		while (ret < 0 && errno == EINTR);

		if (ret < 0)
			return(ret);
		
		buf  += ret;
		size -= ret;
		
		received += ret;
    }
	while (ret==(int)bulk.len && size>0);
	
	return(received);
}

int pusb_endpoint_read(pusb_endpoint_t ep, 
			unsigned char* buf, int size, int timeout)
{
	if (timeout == 0)
		return(pusb_endpoint_rw_no_timeout(ep->fd, ep->ep|USB_DIR_IN, buf, size));
	return(pusb_endpoint_rw(ep->fd ,ep->ep|USB_DIR_IN, buf, size, timeout));
}

int pusb_endpoint_write(pusb_endpoint_t ep, 
			const unsigned char* buf, int size, int timeout)
{
	if (timeout == 0)
		return(pusb_endpoint_rw_no_timeout(
				ep->fd,
				ep->ep | USB_DIR_OUT,
				(unsigned char*) buf, /* Not supposed to be filled */
				size));
	
	return(pusb_endpoint_rw(
			ep->fd,
			ep->ep | USB_DIR_OUT,
			(unsigned char*)buf, /* Not supposed to be filled */
			size,
			timeout));
}

int pusb_endpoint_submit_read (pusb_endpoint_t ep, unsigned char* buf,
							   int size, int signr)
{
	struct usbdevfs_urb* purb;
	int ret;
/*
	printf("submit BULK read %p, len %d\n", buf, size);
*/
	purb = (struct usbdevfs_urb*)malloc(sizeof(struct usbdevfs_urb));
	if (purb == NULL)
		return(-1);
	
	memset(purb, 0, sizeof(struct usbdevfs_urb));
	purb->type = USBDEVFS_URB_TYPE_BULK;
	purb->endpoint = ep->ep | USB_DIR_IN;	
	purb->flags  = USBDEVFS_URB_QUEUE_BULK;
	purb->buffer = buf;
	purb->buffer_length = size;
	purb->signr =  signr;

	do
	{
		ret = ioctl(ep->fd, USBDEVFS_SUBMITURB, purb);
	}
	while (ret < 0 && errno == EINTR);

	if (ret < 0)
		free(purb);

	return(ret);
}

int pusb_endpoint_submit_write(pusb_endpoint_t ep, unsigned char* buf,
							   int size, int signr)
{
	struct usbdevfs_urb* purb;
	int ret;
/*
	printf("URB xxx: BULK/INT endpoint=%d size=%d\n", ep->ep, size);
	dump(buf, size);
*/
	purb = (struct usbdevfs_urb*)malloc(sizeof(struct usbdevfs_urb));
	if (purb == NULL)
		return(-1);
	
	memset(purb, 0, sizeof(struct usbdevfs_urb));
	purb->type = USBDEVFS_URB_TYPE_BULK;
	purb->endpoint = ep->ep | USB_DIR_OUT;	
	purb->flags  = USBDEVFS_URB_QUEUE_BULK;
	purb->buffer = buf;
	purb->buffer_length = size;
	purb->signr =  signr;

	do
	{
		ret = ioctl(ep->fd, USBDEVFS_SUBMITURB, purb);
	}
	while (ret < 0 && errno == EINTR);

	if (ret < 0)
		free(purb);

	return(ret);
}

int pusb_endpoint_submit_int_read (pusb_endpoint_t ep, unsigned char* buf,
								   int size, int signr)
{
	struct usbdevfs_urb* purb;
	int ret;

/*	printf("submit INT read %p, len = %d\n", buf, size);*/

	purb = (struct usbdevfs_urb*)malloc(sizeof(struct usbdevfs_urb));
	if (purb == NULL)
		return(-1);
	
	memset(purb, 0, sizeof(struct usbdevfs_urb));

	purb->type = USBDEVFS_URB_TYPE_BULK; /* INTERRUPT is not supported,
											and BULK works with ep int, so */
	purb->endpoint = ep->ep | USB_DIR_IN;
	purb->flags  = USBDEVFS_URB_QUEUE_BULK;
	purb->buffer = buf;
	purb->buffer_length = size;
	purb->signr =  signr;

	do
	{
		ret = ioctl(ep->fd, USBDEVFS_SUBMITURB, purb);
	}
	while (ret < 0 && errno == EINTR);

	if (ret < 0)
		free(purb);

	return(ret);
}

/* buf is at least pkt_size*pkt_nb bytes */
int pusb_endpoint_submit_iso_read(pusb_endpoint_t ep, unsigned char* buf,
								  int pkt_size, int pkt_nb, int signr)
{
	struct usbdevfs_urb* purb;
	int i, ret;

/*
	printf("submit ISO read ep=%d buf=%p len=%d\n", ep->ep, buf, pkt_size * pkt_nb);
*/
	
	purb = (struct usbdevfs_urb*)malloc(sizeof(struct usbdevfs_urb)
				 + pkt_nb* sizeof(struct usbdevfs_iso_packet_desc));
	
	if (purb == NULL)
		return(-1);
	
	memset(purb, 0, sizeof(struct usbdevfs_urb)
		   + pkt_nb * sizeof(struct usbdevfs_iso_packet_desc));
	
	purb->type = USBDEVFS_URB_TYPE_ISO;
	purb->endpoint = ep->ep | USB_DIR_IN;	
	purb->flags  = USBDEVFS_URB_ISO_ASAP;
	purb->buffer = buf;
	purb->buffer_length = pkt_size * pkt_nb;
	purb->signr =  signr; 
	purb->start_frame = 0;
	purb->number_of_packets = pkt_nb;

	for (i=0;i<pkt_nb;i++) 
		purb->iso_frame_desc[i].length = pkt_size;
	
	do
	{
		ret = ioctl(ep->fd, USBDEVFS_SUBMITURB, purb); 
	}
	while (ret < 0 && errno == EINTR);

	if (ret < 0)
		free(purb);

	return(ret);
}

/*
  Returns a valid URB or NULL
*/

pusb_urb_t pusb_device_get_urb(pusb_device_t dev)
{
	int ret;
	struct usbdevfs_urb* purb;
	pusb_urb_t urb;
	int i, offset, retry;

	retry=0;
	do
	{
		do
		{
			ret = ioctl(dev->fd, USBDEVFS_REAPURB /* NDELAY */, &purb);
		}
		while (ret < 0 && errno == EINTR);

		if (ret < 0 && errno == EAGAIN)
			/* ok, there was no URB waiting for us */
			return NULL;

		/* si ioctl echoue, je retourne -1 code d'erreur dans errno	*/
		if (ret<0)
		{
			perror("ioctl USBDEVFS_REAPURB");
			return(NULL);
		}

		if (purb->status == -84 /* USB_ST_CRC */)
			printf("CRC error in URB\n");

        /* If we get a ctc error then retry, up to 5 times though only
			I've only needed 1 retry so far */
        retry++;
	}
	while (purb->status == -84 /* USB_ST_CRC */&& retry <=5);
	
	if (purb->status !=0 || purb->error_count != 0)
	{
		printf("ep=%02x status=%d error_count=%d\n", purb->endpoint,
			   purb->status, purb->error_count);
		return(NULL);
	}

	urb = (pusb_urb_t) malloc(sizeof(struct pusb_urb_t));
	if (urb == NULL)
	{
		perror("malloc");
		return(NULL);
	}

	urb->ep = purb->endpoint;

	switch (purb->type)
	{
		case USBDEVFS_URB_TYPE_ISO:
			urb->buf_nb = purb->number_of_packets;
			offset = 0;
			for (i=0;i<purb->number_of_packets && i<MAXBUFFER;i++)
			{
				urb->buf[i] = (unsigned char*)purb->buffer + offset;
				urb->buf_size[i] = purb->iso_frame_desc[i].actual_length;
	/*
				printf("size [%d] = %p / %d\n", i, urb->buf[i], urb->buf_size[i]);
	*/
				offset += purb->iso_frame_desc[i].length;
			}
			break;

		case USBDEVFS_URB_TYPE_INTERRUPT:
		case USBDEVFS_URB_TYPE_BULK:
	/*
			if (purb->endpoint & USB_DIR_IN)
			{
				printf("URB xxx : BULK/INT in endpoint=%d size=%d\n",
					   purb->endpoint, purb->actual_length);
				dump(purb->buffer, purb->actual_length);
			}
	*/
			/* no break here */

		case USBDEVFS_URB_TYPE_CONTROL:
			urb->buf_nb = 1;
			urb->buf[0] = purb->buffer;
			urb->buf_size[0] = purb->actual_length;
			break;

		default:
			printf("unsupported URB\n");
			urb->buf_nb = 0;
	}

	/* free it, as it's allocated in pusb_endpoint_[iso_]read */
	free(purb);
	return(urb);
}


int pusb_endpoint_reset(pusb_endpoint_t ep)
{
	return(ioctl(ep->fd, USBDEVFS_RESETEP, &ep->ep));
}

int pusb_endpoint_close(pusb_endpoint_t ep)
{
	/* nothing to do on the struct content */
	free(ep);
	return(0);
}

int pusb_claim_interface(pusb_device_t dev, int interface)
{
	int ret;

	ret = ioctl(dev->fd, USBDEVFS_CLAIMINTERFACE, &interface);
	return(ret);
}

int pusb_release_interface(pusb_device_t dev, int interface)
{
	int ret;

	ret = ioctl(dev->fd, USBDEVFS_RELEASEINTERFACE, &interface);
	return(ret);
}

int pusb_urb_get_epnum(pusb_urb_t urb)
{
	return(urb->ep);
}

int pusb_urb_buffer_first(pusb_urb_t urb,
						  unsigned char** pbuf, int* psize, int* pidx)
{
	*pidx = 0;
	return(pusb_urb_buffer_next(urb, pbuf, psize, pidx));
}

int pusb_urb_buffer_next(pusb_urb_t urb,
						 unsigned char** pbuf, int* psize, int* pidx)
{
	int idx = *pidx;

	if (idx < urb->buf_nb && idx < MAXBUFFER)
	{
		*pbuf = urb->buf[idx];
		*psize = urb->buf_size[idx];
		(*pidx) ++;
		return(1);
	}

	return(0);
}
