/*****************************************************************************
*                                                                            *
*     Driver pour le modem ADSL ECI HiFocus utilise par France Telecom       *
*                                                                            *
*     Author : Valette Jean-Sebastien <jean-sebastien.valette@libertysur.fr  *
*              Sebastien helleu  <flashcode@free.fr>                         *
*                                                                            *
*     Copyright : GPL                                                        *
*                                                                            *
*                                                                            *
*****************************************************************************/

/*
		Include stuf
*/

#include <linux/module.h>
#include <linux/modversions.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/malloc.h>
#include <linux/errno.h>

#include <linux/usb.h>

#include <linux/atm.h>
#include <linux/atmdev.h>


/*
		Version definition
*/
#define ECI_VERSION 0
#define ECI_REVISION 1


/*
		Debuging Stuf
*/

#define DEBUG 1
#ifdef DEBUG
#define DBG_OUT(X) printk(KERN_DEBUG "ECI_USB :  " X )
#else
#define DBG_OUT(X) 
#endif

/*
	Constants
*/
#define ECI_WAN_VENDOR_ID 0x0915	
#define ECI_WAN_PRODUCT_ID 0x8000	

#define ECI_INT_EP  0x86
#define ECI_ISO_PIPE 0x88

#define ECI_NB_ISO_PACKET 10
#define ECI_ISO_PACKET_SIZE 448
#define ECI_ISO_BUFFER_SIZE (ECI_ISO_PACKET_SIZE * ECI_NB_ISO_PACKET)

/*
	Globals	
*/
static const char eci_drv_name[] =  "ECI HiFocus ADSL Modem";
static const char eci_drv_label[] =  "ECI";
static const unsigned char eci_init_setup_packets[] =
{
#include "usb_packets.h"
};


/*
		USB stuff
*/


static const struct usb_device_id eci_usb_deviceids[] =
{
	{ USB_DEVICE ( ECI_WAN_VENDOR_ID , ECI_WAN_PRODUCT_ID ) } ,
	{ } 
};

/*
	ATM stuff
*/
static int eci_atm_open(struct atm_vcc *vcc,short vpi, int vci);
static void eci_atm_close(struct atm_vcc *vcc);
static int eci_atm_ioctl(struct atm_dev *vcc,unsigned int cmd, void *arg);
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb);
static void eci_init_vendor_callback(struct urb *urb);
static void eci_ep_int_callback(struct urb *urb);
static void eci_iso_callback(struct urb *urb);


static const struct atmdev_ops eci_atm_devops =
{
	open: eci_atm_open,
	send: eci_atm_send,
	close: eci_atm_close,
	ioctl:	eci_atm_ioctl
};


/*	
		Module Stuff
*/
#ifdef MODULE
MODULE_DEVICE_TABLE ( usb , eci_usb_deviceids ) ;

/*	prototypes	*/
static int __init eci_init(void);
static void __exit eci_cleanup(void);

MODULE_AUTHOR( "Sebastien helleu, Jean-Sebastien Valette" );
MODULE_DESCRIPTION( "ECI HiFocus ADSL Modem Driver");
module_init (eci_init);
module_exit (eci_cleanup);


#endif
/*
	USB Driver stuff
*/
/*	prototypes	*/	
void *eci_usb_probe(struct usb_device *dev,unsigned int ifnum , 
	const struct usb_device_id *id);
void eci_usb_disconnect(struct usb_device *dev, void *p);
int eci_usb_ioctl(struct usb_device *usb_dev,unsigned int code, void * buf);


/*	structures	*/

struct eci_instance 		/*	Private data for driver	*/
{
	struct eci_instance	*next;		
	struct usb_device 	*usb_wan_dev;	
	struct atm_dev		*atm_dev;
	int 				minor;	
	wait_queue_head_t 	eci_wait; 
	unsigned char           *setup_packets;
	struct urb		*interrupt_urb;
	unsigned char		interrupt_buffer[32];
	struct urb		*isourbs;
};

struct eci_instance	*eci_instances= NULL;		/*  Driver list	*/


struct usb_driver eci_usb_driver = {
	name:		eci_drv_name,
	id_table:	eci_usb_deviceids,
	probe:		eci_usb_probe,
	disconnect:		eci_usb_disconnect
/*	minor:		ECI_MINOR_START,	usb charddev stuff
	fops:		eci_file_ops,		may be in future :) 
	ioctl:		eci_usb_ioctl */
};


/*	
		Driver initialisation 

*/


static int __init eci_init(void)
{	
	/*	Must register driver and claim for hardware */
	DBG_OUT("init module in\n");
	printk("ECI HiFocus ADSL Modem Driver loading\n");
	DBG_OUT("init module out\n");
	return (usb_register(&eci_usb_driver));

}


static void __exit eci_cleanup(void)
{
	DBG_OUT("cleanup in\n");
	usb_deregister(&eci_usb_driver);
	return;
	DBG_OUT("cleanup out\n");
};
	
void *eci_usb_probe(struct usb_device *dev,unsigned int ifnum , 
	const struct usb_device_id *id)
{
	struct eci_instance *out_instance= NULL;
	struct urb *eciurb, *tmpurb;
	unsigned char *setuppacket;
	int size;
	int dir;
	int eci_isourbcnt;
	int i;	/*	loop counter	*/


	DBG_OUT("Probe in\n");
/*		We don't yet support more than one modem connected */
	
/************************************************

	The code Here will have to initialize modem 
	then claim for wan usb device

************************************************/								
	if(dev->descriptor.bDeviceClass == USB_CLASS_VENDOR_SPEC ||
	   dev->descriptor.idVendor == ECI_WAN_VENDOR_ID ||
	   dev->descriptor.idProduct == ECI_WAN_PRODUCT_ID )
	{
		out_instance= kmalloc(sizeof(struct eci_instance), 
							GFP_KERNEL);
		if(out_instance)
		{
			eci_instances=out_instance; 
			out_instance->usb_wan_dev= dev;
			out_instance->setup_packets = eci_init_setup_packets;
			init_waitqueue_head(&out_instance->eci_wait);
		}
		else
		{
			printk(KERN_ERR " out of memory\n");
			return 0;
		}
		DBG_OUT("Setting configuration 1\n");
		if(usb_set_configuration(dev,1)<0)
		{
			printk(KERN_ERR " eci_usb: Can't set interface\n");
			return 0;
		}
		DBG_OUT("Setting interface\n");
		if(usb_set_interface(dev,0,4)<0)
		{
			printk(KERN_ERR " eci_usb: Cant set configuration\n");
			return 0;
		}
		DBG_OUT("Init ISO urbs\n");
		out_instance->isourbs = 0;
		for(eci_isourbcnt=0;eci_isourbcnt<20;eci_isourbcnt++)
		{
			if(!(eciurb=usb_alloc_urb(ECI_NB_ISO_PACKET)))
			{
				printk(KERN_ERR 
			" eci_usb: not enought memory for iso urb %d\n",
 					eci_isourbcnt);
				return 0;
			}
			if(!(eciurb->transfer_buffer=kmalloc(
					ECI_ISO_BUFFER_SIZE, GFP_KERNEL)))
			{
				printk(KERN_ERR 
			" eci_usb: not enought memory for iso buffer %d\n",
 					eci_isourbcnt);
				return 0;
			}
			eciurb->next = out_instance->isourbs;
			out_instance->isourbs = eciurb;
			eciurb->dev = dev;
			eciurb->context = out_instance;
			eciurb->pipe = usb_rcvisocpipe(dev, ECI_ISO_PIPE);
			eciurb->transfer_flags = USB_ISO_ASAP;
			eciurb->number_of_packets = ECI_NB_ISO_PACKET;
			eciurb->complete = eci_iso_callback;
			eciurb->transfer_buffer_length = ECI_ISO_BUFFER_SIZE;
			for(i=0;i < ECI_NB_ISO_PACKET; i ++)
			{
				eciurb->iso_frame_desc[i].offset = 
					i * ECI_ISO_PACKET_SIZE;
				eciurb->iso_frame_desc[i].length = 
					ECI_ISO_PACKET_SIZE;
			}
		}
		tmpurb = out_instance->isourbs;
		while(tmpurb->next) tmpurb=tmpurb->next;
		tmpurb->next = out_instance->isourbs;
		if(usb_submit_urb(eciurb))
		{
			printk( KERN_ERR "error couldn't send iso urbs\n");
			return 0;
		}
		/*
			Should reset the Iso EP and send 20 Urbs
		*/
		DBG_OUT("Init INT endpoint\n");
		if(!(eciurb=usb_alloc_urb(0)))
		{
			printk(KERN_ERR "Can't allocate int urb !\n");
			return 0;
		}
	/*	interval tacken from cat /proc/bus/usb/devices values	*/
		out_instance->interrupt_urb = eciurb;
		FILL_INT_URB(eciurb, dev, usb_rcvintpipe(dev, ECI_INT_EP), 
			&out_instance->interrupt_buffer,32,
			eci_ep_int_callback,out_instance,3);
		if(usb_submit_urb(eciurb))
		{
			printk( KERN_ERR "error couldn't send interrupt urb\n");
			return 0;
		}
		/*
			Should reset the Iso EP and send 20 Urbs
		*/
		/*DBG_OUT("Bulk Reset\n");*/
		/*
			Just Reset EP 0x02 (no reset)
			if some one day, all endpoint before sending any urb
		*/
		DBG_OUT("Vendor Stuff\n");
		/*
			Should send vendors URB here after reseting pipe 0x86
			First allocate the urb struct
		*/
		eciurb=usb_alloc_urb(1);
		if(!eciurb)
		{
			printk(KERN_ERR " Can't allocate urb\n");
			return 0;
		};
		memset(eciurb, 0, sizeof(struct urb));
		/*
			Next allocate buffer for data transmition.
			Last 8 byte will be reserved for setupackets
		*/
		if(!(eciurb->transfer_buffer = kmalloc((64*1024), GFP_KERNEL)))
		{
			printk(KERN_ERR " out of memory\n");
			return 0;
		};
		/*
			Initialise all urb struct
		*/
		setuppacket = eciurb->transfer_buffer +  64 * 1024 -8;
		memcpy(setuppacket, out_instance->setup_packets,8);
		size = (out_instance->setup_packets[7] << 8 ) |
			out_instance->setup_packets[6];
#ifdef DEBUG
		printk(KERN_DEBUG "size = %d\n", size);
		printk( KERN_DEBUG 
		"SetupPacket : %02x %02x %02x %02x %02x %02x %02x %02x\n", 
		setuppacket[0], setuppacket[1], setuppacket[2],
		setuppacket[3], setuppacket[4], setuppacket[5],
		setuppacket[6], setuppacket[7]);
#endif
		/*
			If write URB then read the buffer
		*/
		if(!(setuppacket[0] & 0x80))
		{
			memcpy(eciurb->transfer_buffer, 
				out_instance->setup_packets+8,size);
		}

		FILL_CONTROL_URB(eciurb, dev,usb_rcvctrlpipe(dev,0), 
				setuppacket, eciurb->transfer_buffer, 
				size, eci_init_vendor_callback,
				out_instance);
		DBG_OUT("Sending URB\n");
		if(usb_submit_urb(eciurb))
		{
			printk( KERN_ERR "error couldn't send init urb\n");
			return 0;
		}
		DBG_OUT("URB Sent \n");
	}				
	DBG_OUT("Probe: done with usb, doing ATM\n");
	if((out_instance->atm_dev = atm_dev_register(eci_drv_label,
					&eci_atm_devops , -1, NULL)))
	{
		out_instance->atm_dev->dev_data = (void *)out_instance;
		out_instance->atm_dev->ci_range.vpi_bits = ATM_CI_MAX;
		out_instance->atm_dev->ci_range.vci_bits = ATM_CI_MAX;
		out_instance->atm_dev->link_rate = 640*1024/8/53;
/*	Further ATM DEVICE Should be initialize here	*/
		DBG_OUT("Probe: ATM device registered\n");
	}	
	else
	{
		printk(KERN_ERR "Can't init ATM device\n");
	}
	printk(KERN_DEBUG "out_instance : %x\n" ,out_instance);
	MOD_INC_USE_COUNT;
	DBG_OUT("Probe out\n");
	return out_instance;
};

void eci_usb_disconnect(struct usb_device *dev, void *p)
{
	struct urb *urb;
	DBG_OUT("disconnect in\n");
	MOD_DEC_USE_COUNT;
	if(eci_instances)
	{
		DBG_OUT("disconnect : freeing atm_dev\n");
		atm_dev_deregister(eci_instances->atm_dev);
		DBG_OUT("disconnect : freeing int urb\n");
		usb_unlink_urb(eci_instances->interrupt_urb);
		usb_free_urb(eci_instances->interrupt_urb);
		DBG_OUT("disconnect : freeing iso urbs\n");
		urb=eci_instances->isourbs;
		while(urb->next!=eci_instances->isourbs) urb=urb->next;
		urb->next = 0;
		while(urb=eci_instances->isourbs)
		{
			DBG_OUT("Freeing one iso\n");
			eci_instances->isourbs = urb->next;
			usb_unlink_urb(urb);
			usb_free_urb(urb);
		}
		printk(KERN_DEBUG "disconnect : freeing instance %x\n", eci_instances);
		kfree(eci_instances);
		eci_instances=NULL;
	}
	DBG_OUT("disconnect out\n");

};


int eci_usb_ioctl(struct usb_device *usb_dev,unsigned int code, void * buf)
{
	DBG_OUT("eci_usb_close in\n");
	DBG_OUT("eci_usb_close out\n");
	return -1;
};

/*******************************************************************************
		
		ATM functions

*******************************************************************************/
static int eci_atm_open(struct atm_vcc *vcc,short vpi, int vci)
{
	DBG_OUT("eci_atm_open in\n");
	
	DBG_OUT("eci_atm_open out\n");
	return 0;
};
static void eci_atm_close(struct atm_vcc *vcc)
{
	DBG_OUT("eci_atm_close in\n");
	DBG_OUT("eci_atm_close out\n");
};
static int eci_atm_ioctl(struct atm_dev *vcc,unsigned int cmd, void *arg)
{
	DBG_OUT("eci_atm_ioctl in\n");
	DBG_OUT("eci_atm_ioctl out\n");
	return -1;
};
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb)
{
	DBG_OUT("eci_atm_send in\n");
	DBG_OUT("eci_atm_send out\n");
	return -1;
};



/*******************************************************************************
		
		USB functions

*******************************************************************************/

/*
		This one is called to handle the modem sync.

		It must send the saved URB and checked for returns value

		Based on ECI-LOAD2 by benoit papillault

		Valette Jean-Sebastien 

		30 11 2001

*/
static void eci_init_vendor_callback(struct urb *urb)
{
	struct eci_instance *instance;
	struct usb_device	*dev;
	int size;
	int i;
	unsigned char *buffer;

	DBG_OUT("eci: init callback called !\n");
	instance = (struct eci_instance *) urb->context;
	dev = instance->usb_wan_dev;
	printk(KERN_DEBUG "ECI_USB: dev = %x\n",dev);

/*
	If urb status is set warn about it, else do what we gotta do
*/

	if(urb->status)
		printk(KERN_ERR "eci: Urb with status %d\n" , urb->status);
	else
	{
		/*	Check For datarecived and wanted data length */
		if( urb->actual_length != urb->transfer_buffer_length )
			printk(KERN_WARNING "eci: expecting %d, got %d\n",
				urb->transfer_buffer_length, 
				urb->actual_length );

#ifdef DEBUG
		if ((urb->setup_packet[0] & 0x80))
		{
			printk(KERN_DEBUG "ECI_USB:");
			size = ((instance->setup_packets[7]<<8) | 
					instance->setup_packets[6]);
			buffer = (char *) urb->transfer_buffer;
			for (i=0;i<size;i++)
				printk( "%02x ",
					buffer[i]);
			printk("\n");
		}
#endif	/*	DEBUG	*/
		if ( !((urb->setup_packet[0] & 0x40) == 0x40 && urb->setup_packet[1]== 0xdc
			&&((urb->setup_packet[3] * 256 + urb->setup_packet[2])== 0x00) && 
			((urb->setup_packet[5] * 25 + urb->setup_packet[4]) == 0x00)))
		{
			/*	
				sould send next VENDOR URB
			*/

			instance = urb->context;
/* 
			Old buffer size
*/
			size = (urb->setup_packet[7] << 8 ) |
				urb->setup_packet[6]; 
/*
	if it was a write control then increment pointer with 
	setup packet size and buffer size, else just setup packet size
*/
			instance->setup_packets += 
				(urb->setup_packet[0] & 0x80) ? 8 : + size;
/*
		New buffer size
*/
			size = (instance->setup_packets[7] << 8 ) |
				instance->setup_packets[6];
			memcpy(urb->setup_packet, instance->setup_packets,8);
#ifdef DEBUG
		printk(KERN_DEBUG "size = %d\n", size);
		printk( KERN_DEBUG 
		"SetupPacket : %02x %02x %02x %02x %02x %02x %02x %02x\n", 
		urb->setup_packet[0], urb->setup_packet[1],
		urb->setup_packet[2], urb->setup_packet[3],
		urb->setup_packet[4], urb->setup_packet[5],
		urb->setup_packet[6], urb->setup_packet[7]);
#endif
			if(!(urb->setup_packet[0] & 0x80))
			{
				memcpy(urb->transfer_buffer, 
					instance->setup_packets+8,size);
				buffer = urb->transfer_buffer;
				printk(KERN_DEBUG "ECI:");
				for(i=0; i< size;i++)
				{
				printk(KERN_DEBUG "0x%02x",buffer[i]);
				}
				printk("\n");
			}
			FILL_CONTROL_URB(urb, dev,usb_rcvctrlpipe(dev,0), 
				urb->setup_packet, urb->transfer_buffer, 
				size, eci_init_vendor_callback,
				instance);
			DBG_OUT(KERN_DEBUG "Resending URB\n");
			if(usb_submit_urb(urb))
			{
				printk( KERN_ERR 
					"error couldn't resend init urb\n");
			}
		}
#ifdef DEBUG
		else
			DBG_OUT("Waiting for data on 0x86 endpoint\n");
#endif DEBUG
	}
	DBG_OUT("Vendor callBack out\n");
}

/*

	Handle The interrupt stuff

	if 
		data length == 32 we are after initialization state
	else 
		if we have expected data
			send next vendor

	Valette Jean-Sebastien 

	30 11 2001
	

*/
static void eci_ep_int_callback(struct urb *urb)
{
	DBG_OUT("Int callBack in\n");
	if(urb->actual_length)
	{
		if(urb->actual_length!=32)
		{	/*		Synchronisation 
					Check Data returned by int urb
					and send Next Vendor
			*/
#ifdef DEBUG
	printk(KERN_DEBUG "EP int received %d bytes\n", urb->actual_length);
#endif	
		}
		else
		{	/*	Already synchronized just answer	*/
			DBG_OUT("EP int received 32 bytes\n");
		}
	}
#ifdef DEBUG
	else
	{
		DBG_OUT("Received int with no data\n");
	}
#endif // DEBUG
	DBG_OUT("Int callBack out\n");
}

static void eci_iso_callback(struct urb *urb)
{
	struct eci_instance *instance;
	int i;

	DBG_OUT("Iso Callback Called\n");
	instance = (struct eci_instance *)urb->context;
	urb->dev = instance->usb_wan_dev;
	urb->pipe = usb_rcvisocpipe(urb->dev, ECI_ISO_PIPE);
	urb->transfer_flags = USB_ISO_ASAP;
	urb->number_of_packets = ECI_NB_ISO_PACKET;
	urb->complete = eci_iso_callback;
	urb->transfer_buffer_length = ECI_ISO_BUFFER_SIZE;
	for(i=0;i < ECI_NB_ISO_PACKET; i ++)
	{
		urb->iso_frame_desc[i].offset = i * ECI_ISO_PACKET_SIZE;
		urb->iso_frame_desc[i].length = ECI_ISO_PACKET_SIZE;
	}
/*	if(usb_submit_urb(urb))
	{
		printk( KERN_ERR 
			"error couldn't resend iso urb\n");
	}*/
	DBG_OUT("Iso Callback Exit\n");
}
