/*
*  ALCATEL SpeedTouch USB : Portable USB user library -- Linux implementation
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
*  $Id$


Design for pusb-linux.c:

pusb_endpoint_read/write:

  -> submit_urb [ioctl fdusb USBDEVFS_SUBMITURB]
  -> wait on internal library [pthread_mutex_lock? semaphore_decr?]

wait : 
  poll(usb)
  readpurb() [ioctl fdusb USBDEVFS_REAPURB]
  wake_up_wait_depending_on_urb() [pthread_mutex_unlock? semaphore_incr?]

poll(fdusb) returns POLLOUT if an async request has completed
  and the file has been opened in WRITE mode (O_WRONLY or O_RDWR)

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/utsname.h>

#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <asm/page.h>

#include "pusb.h"

/******************************************************************************
*	Structures
******************************************************************************/

struct pusb_device_info
{
    char * file;
    int dev_class;
    int dev_subclass;
    int dev_protocol;
    int dev_vendor;
    int dev_product;
    int dev_version;
};

struct pusb_enum
{
    int current;
    int n_path;
    struct pusb_device_info * info;
};

struct pusb_device
{
	int fd;
    int is_2_5;
};

struct pusb_endpoint
{
	int fd;
    int is_2_5;
	int ep;
};

static const char usb_path[] = "/proc/bus/usb";

/* Device descriptor */
struct usb_device_descriptor {
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

/*****************************************************************************
*	Local Prototypes
*****************************************************************************/

static int test_file(const char *path, int vendorID, int productID);
static int usbfs_search(const char *path, int vendorID, int productID);
static pusb_device_t make_device(int fd);

/*****************************************************************************
*	Library functions
*****************************************************************************/


static void wait_ms(int ms)
{
    struct timeval tv;
    int r;

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms - 100 * tv.tv_sec) * 1000;

    r = select(FD_SETSIZE, NULL, NULL, NULL, &tv);
}

/*
  read a word in little endian
*/

static unsigned short read_word(unsigned short * w)
{
    unsigned char * ptr = (unsigned char *) w;
    return ( ((unsigned short)ptr[0]) | ((unsigned short)ptr[1]<<8) );
}

/*
  pusb_test_file : test if the given file points to a USB device
  and returns 1 in this case. Otherwise, returns 0.

  Fill in pusb_device_info, except file.
*/

static int pusb_test_file(const char * path, struct pusb_device_info * info)
{
	int fd;
	struct usb_device_descriptor desc;
    int result = 0;
	
	if ((fd = open(path, O_RDWR)) == -1)
    {
        if (errno != -ENOENT)
            perror(path);
        return 0;
	}
  
	if (read(fd,&desc,sizeof(desc)) == sizeof(desc))
    {
		/*
         * Great, we read something
         * check, it match the correct structure
         */
        
		if(desc.bLength == sizeof(desc))
        {
              info->dev_class    = desc.bDeviceClass;
              info->dev_subclass = desc.bDeviceSubClass;
              info->dev_protocol = desc.bDeviceProtocol;
              info->dev_vendor   = read_word(&desc.idVendor);
              info->dev_product  = read_word(&desc.idProduct);
              info->dev_version  = read_word(&desc.bcdDevice);
              result = 1;
		}

	}
	
	close(fd);

    return result;
}

/*
  Returns 0 on successs, -1 on failure
*/

static int pusb_scan_dir(const char * path, pusb_enum_t en)
{
	DIR * dir;
	struct dirent * dirp;
	
	if ((dir = opendir(path)) == NULL)
    {
		perror(path);
		return -1;
	}
	
	while ((dirp=readdir(dir)) != NULL)
    {

		struct stat statbuf;
		char file[PATH_MAX+1];
		
		if (strlen(dirp->d_name) != 3)
			continue;

		if (!isdigit(dirp->d_name[0]) ||
		    !isdigit(dirp->d_name[1]) ||
		    !isdigit(dirp->d_name[2]))
			continue;

		sprintf(file,"%s/%s",path,dirp->d_name);
		
		if (stat(file,&statbuf) != 0)
        {
			perror(file);
			continue;
		}
		
		if (S_ISDIR(statbuf.st_mode))
        {
            pusb_scan_dir(file, en);
            continue;
		}
		
		if (S_ISREG(statbuf.st_mode))
        {
            struct pusb_device_info info;

            if (pusb_test_file(file, &info))
            {
                /* add this USB device to the list */

                en->info = (struct pusb_device_info *) realloc(
                    en->info, sizeof(struct pusb_device_info)*(en->n_path+1));
                if (en->info == NULL)
                {
                    /* need to handle the error */
                }

                info.file = strdup(file);
                en->info[en->n_path++] = info;
            }
		}

	}
	
	closedir(dir);

    return 0;
}

int pusb_enum_init(pusb_enum_t * ref)
{
    /* allocate memory for *ref */

    *ref = (struct pusb_enum *) malloc( sizeof(struct pusb_enum));
    if (*ref == NULL)
        return 0;

    /* initialize *ref */

    (*ref)->current = -1;
    (*ref)->n_path  = 0;
    (*ref)->info    = NULL;

    if (pusb_scan_dir(usb_path, *ref) < 0)
    {
        free (*ref);
        return 0;
    }

    return 1;
}

int pusb_enum_next(pusb_enum_t * ref)
{
    if ((*ref)->current + 1 < (*ref)->n_path)
    {
        (*ref)->current ++;
        return 1;
    }

    return 0;
}

void pusb_enum_done(pusb_enum_t * ref)
{
    int i;

    for (i=0;i<(*ref)->n_path;i++)
        free ((*ref)->info[i].file);
    free ((*ref)->info);

    free (*ref);
}

int pusb_get_device_class(pusb_enum_t * ref)
{
    if ((*ref)->current < (*ref)->n_path)
        return (*ref)->info[(*ref)->current].dev_class;
    return -1;
}

int pusb_get_device_subclass(pusb_enum_t * ref)
{
    if ((*ref)->current < (*ref)->n_path)
        return (*ref)->info[(*ref)->current].dev_subclass;
    return -1;
}

int pusb_get_device_protocol(pusb_enum_t * ref)
{
    if ((*ref)->current < (*ref)->n_path)
        return (*ref)->info[(*ref)->current].dev_protocol;
    return -1;
}

int pusb_get_device_vendor(pusb_enum_t * ref)
{
    if ((*ref)->current < (*ref)->n_path)
        return (*ref)->info[(*ref)->current].dev_vendor;
    return -1;
}

int pusb_get_device_product(pusb_enum_t * ref)
{
    if ((*ref)->current < (*ref)->n_path)
        return (*ref)->info[(*ref)->current].dev_product;
    return -1;
}

int pusb_get_device_version(pusb_enum_t * ref)
{
    if ((*ref)->current < (*ref)->n_path)
        return (*ref)->info[(*ref)->current].dev_version;
    return -1;
}

pusb_device_t pusb_device_open(pusb_enum_t * ref)
{
    const char * file;
    int fd;

    if ((*ref)->current < (*ref)->n_path)
    {
        file = (*ref)->info[(*ref)->current].file;
        fd = open(file,O_RDWR);
        if (fd < 0)
            return NULL;

        return make_device(fd);
    }

    return NULL;
}

int pusb_device_close(pusb_device_t dev)
{
	int ret;

	ret = close(dev->fd);
	free(dev);

	return(ret);
}

/*
* Function     : pusb_control_msg
* Return value : ioctl returned value (see linux usb docs)
* Description  : sends a control msg urb to the device
*/
int pusb_control_msg(	pusb_device_t dev,
			int request_type,
			int request,
			int value,
			int index, 
			unsigned char *buf,
			int size)
{

	int ret;
	struct usbdevfs_ctrltransfer ctrl;
    int timeout = 1000;

	ctrl.requesttype = request_type;
	ctrl.request     = request;
	ctrl.value       = value;
	ctrl.index       = index;
	ctrl.length      = size;
	ctrl.timeout     = timeout;
	ctrl.data        = buf;

	ret = ioctl(dev->fd,USBDEVFS_CONTROL,&ctrl);

	return(ret);

}

/*
* Function     : pusb_set_configuration
* Return value : ioctl returned value (see linux-usb docs)
* Description  : cf function name
*/
int pusb_device_set_configuration(pusb_device_t dev, int config)
{

	int ret;

	ret = ioctl(dev->fd,USBDEVFS_SETCONFIGURATION,&config);

	return(ret);

}

/*
* Function     : pusb_set_interface
* Return value : ioctl returned value (see linux-usb docs)
* Description  : see function name
*/
int pusb_device_set_interface(pusb_device_t dev, int interface, int alternate)
{

	struct usbdevfs_setinterface setintf;
	int ret;

	setintf.interface = interface;
	setintf.altsetting = alternate;

	ret = ioctl(dev->fd,USBDEVFS_SETINTERFACE,&setintf);

	return(ret);

}

/*
* Function     : pusb_endpoint_open
* Return value : NULL on error, a valid ep on success
* Description  : see function name
*/
pusb_endpoint_t pusb_endpoint_open(pusb_device_t dev, int epnum)
{

	pusb_endpoint_t ep;

	if ((ep = (pusb_endpoint_t) malloc(sizeof(*ep))) == NULL)
		return(NULL);

	ep->fd = dev->fd;
    ep->is_2_5 = dev->is_2_5;
	ep->ep = epnum & 0xf;

	return(ep);

}

/*
* Function     : pusb_endpoint_rw_no_timeout
* Return value : ioctl returned value
* Description  : Writes or Read from an usb end point (without timeout value)
*/

static int pusb_endpoint_rw_no_timeout(int fd,
				int ep,
				const unsigned char *buf,
				int size)
{
	struct usbdevfs_urb urb, * purb;
	int ret;

	wait_ms(10);

	memset(&urb,0,sizeof(urb));

	urb.type          = USBDEVFS_URB_TYPE_BULK;
	urb.endpoint      = ep;
	urb.flags         = 0;
	urb.buffer        = (unsigned char*)buf;
	urb.buffer_length = size;
	urb.signr         = 0;

	do {
		ret = ioctl(fd,USBDEVFS_SUBMITURB,&urb);
	} while(ret < 0 && errno == EINTR);

	if (ret < 0)
		return(ret);

	do {
		ret = ioctl(fd,USBDEVFS_REAPURB,&purb);
	} while(ret < 0 && errno == EINTR);

	if(ret < 0)
		return(ret);

/*
	if(purb != &urb)
		fprintf(stderr, "purb=%p, &urb=%p\n",purb,&urb);

	if(purb->buffer != buf)
		fprintf(stderr, "purb->buffer=%p, buf=%p\n",purb->buffer,buf);
*/

	return (purb->status < 0) ? purb->status : purb->actual_length;

}

/*
* Function     : pusb_endpoint_rw
* Return value : ioctl returned value
* Description  : Writes or Read from an usb end point (with timeout value)
*/

static int pusb_endpoint_rw(
			int fd,
			int ep,
			const unsigned char * buf,
			int size,
			int timeout)
{

	struct usbdevfs_bulktransfer bulk;
	int ret, received = 0;

    /*
      if timeout == 0, this will cause a zero wait inside the kernel
      and the transfer will always fail. So, we replace timeout == 0
      with a large value
    */

    if (timeout == 0)
        timeout = 0x7fffffff;

	wait_ms(10);

	do {

		bulk.ep      = ep;
		bulk.len     = (size > PAGE_SIZE)?PAGE_SIZE:size;
		bulk.timeout = timeout;
		bulk.data    = (unsigned char*)buf;

		do {
			ret = ioctl(fd,USBDEVFS_BULK,&bulk);
		} while (ret < 0 && errno == EINTR);

		if (ret < 0)
			return(ret);
		
		buf      += ret;
		size     -= ret;
		received += ret;

	} while(ret==bulk.len && size>0);

	return(received);
}

/*
* Function     : pusb_endpoint_close
* Return value : 0
* Description  : Close the end pont given in parameter
*/
int pusb_endpoint_close(pusb_endpoint_t ep)
{
	/* nothing to do on the struct content */
	free(ep);

	return(0);

}

/*****************************************************************************
*	Local functions
*****************************************************************************/

/*
* Function     : test_file
* Return value : -1 on error, a valid filedescriptor on success
* Description  : Try to open the file and get USB device information,
*                if it's ok, check if it matches vendorID & productID
*/
static int test_file(const char *path, int vendorID, int productID)
{

	int fd;
	struct usb_device_descriptor desc;
	
	if((fd = open(path, O_RDWR)) == -1) {
		perror(path);
		return(-1);
	}
  
	if(read(fd,&desc,sizeof(desc)) == sizeof(desc))  {

		/*
		* Great, we read something
		* check, it match the correct structure
		*/
		if(desc.bLength == sizeof(desc)) {

			/*	  
			  fprintf(stderr, "=== %s ===\n",path);
			  fprintf(stderr, "  bLength         = %u\n",desc.bLength);
			  fprintf(stderr, "  bDescriptorType = %u\n",desc.bDescriptorType);
			  fprintf(stderr, "  bcdUSB          = %04x\n",desc.bcdUSB);
			  fprintf(stderr, "  idVendor        = %04x\n",desc.idVendor);
			  fprintf(stderr, "  idProduct       = %04x\n",desc.idProduct);
			  fprintf(stderr, "  bcdDevice       = %04x\n",desc.bcdDevice);
			*/
			if(	vendorID == desc.idVendor &&
				productID == desc.idProduct)
				return(fd);
		}

	}
	
	close(fd);

	return(-1);

}

/*
* Function     : usbfs_search
* Return value : -1 on error, a valid filedescriptor on success
* Description  : Search for a vendorID, productID.
*/
static int usbfs_search(const char *path, int vendorID, int productID)
{

	int result = -1;
	
	DIR * dir;
	struct dirent * dirp;
	
	if((dir = opendir(path)) == NULL) {
		perror(path);
		return(-1);
	}
	
	while((dirp=readdir(dir)) != NULL) {

		struct stat statbuf;
		char file[PATH_MAX+1];
		
		if (strlen(dirp->d_name) != 3)
			continue;

		if (!isdigit(dirp->d_name[0]) ||
		    !isdigit(dirp->d_name[1]) ||
		    !isdigit(dirp->d_name[2]))
			continue;

		sprintf(file,"%s/%s",path,dirp->d_name);
		
		if (stat(file,&statbuf) != 0) {
			perror(file);
			continue;
		}
		
		if (S_ISDIR(statbuf.st_mode)) {

			if((result = usbfs_search(file,vendorID,productID)) < 0)
				continue;
			else
				break;
		}
		
		if (S_ISREG(statbuf.st_mode)) {

			if ((result=test_file(file,vendorID,productID)) < 0)
				continue;
			else
				break;
		}

	}
	
	closedir(dir);

	return(result);

}

/*
  is_kernel_2_5: returns TRUE if the running kernel is 2.5 or 2.6
*/

static int is_kernel_2_5()
{
    struct utsname uts;
    int ret;

    uname(&uts);

    ret = (strncmp(uts.release,"2.5.",4) == 0)
        || (strncmp(uts.release,"2.6.",4) == 0) ;

    return ret;
}

/*
* Function     : make_device
* Return value : NULL on error, a valid pusb_device_t
* Description  : Allocates a pusb_device_t data structure
*/
static pusb_device_t make_device(int fd)
{

	pusb_device_t dev;

	if((dev = malloc(sizeof(*dev))) == NULL) {
		close (fd);
		return(NULL);
	}

    /* check for a 2.5 kernel */
	
    dev->fd = fd;
    dev->is_2_5 = is_kernel_2_5();
	return(dev);

}

int pusb_endpoint_bulk_read(pusb_endpoint_t ep,
                            unsigned char *buf, int size)
{
    return pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_IN,buf,size,0);
}

int pusb_endpoint_bulk_write(pusb_endpoint_t ep,
                             const unsigned char *buf, int size)
{
    return pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_OUT,buf,size,0);
}

int pusb_endpoint_int_read(pusb_endpoint_t ep,
                           unsigned char *buf, int size)
{
    return pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_IN,buf,size,0);
}

int pusb_endpoint_int_write(pusb_endpoint_t ep,
                            const unsigned char *buf, int size)
{
    return pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_OUT,buf,size,0);
}

int pusb_endpoint_iso_read(pusb_endpoint_t ep,
                           unsigned char *buf, int size)
{
    return pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_IN,buf,size,0);
}

int pusb_endpoint_iso_write(pusb_endpoint_t ep,
                            const unsigned char *buf, int size)
{
    return pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_OUT,buf,size,0);
}
