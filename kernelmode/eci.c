/*****************************************************************************
*                                                                            *
*     Driver pour le modem ADSL ECI HiFocus utilise par France Telecom       *
*                                                                            *
*     Author : Valette Jean-Sebastien <jean-sebastien.valette@libertysur.fr  *
*              Eric Bardes  <email@fournisseur>                         *
*                                                                            *
*     Copyright : GPL                                                        *
*                                                                            *
*                                                                            *
*****************************************************************************/

/**********************************************************************
 Project	:	Driver modem ADSL ECI HiFocus
 File		:	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
 **********************************************************************
 Description	:	
***********************************************************************/

/**********************************************************************
                    Include stuf
***********************************************************************/

#include <linux/module.h>
#include <linux/modversions.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/version.h>

/*
 * replaced by slab as advived by gcc
 #include <linux/malloc.h>
 */
#include <linux/slab.h>
#include <linux/errno.h>

#include <linux/usb.h>

#include <linux/atm.h>
#include <linux/atmdev.h>
#include <asm/byteorder.h>

#include <linux/spinlock.h>

#define __USE_ATM__

/**********************************************************************
                    Version definition
***********************************************************************/
#define ECI_VERSION 0
#define ECI_REVISION 1


/**********************************************************************
                    Debuging Stuf
***********************************************************************/

#define DEBUG 1
#ifdef DEBUG

#define DBG_OUT(fmt, argz...) \
	printk( \
		KERN_DEBUG __FILE__ "" fmt, \
		##argz \
	)

#define DUMP_LINES 	100

#define DBG_RAW_OUT(txt, buff, size) \
	_dumpk( \
		txt, \
		__FILE__, \
		__LINE__, \
		buff, \
		size \
	)

/*
 * Dump the content of a sized buffer in syslog kernel/debug
 */
static void _dumpk(
	const char *	txt,	/* IN: Comment		*/
	const char *	file,	/* IN: File name	*/
	int		line,	/* IN: Line number	*/
	unsigned char *	buffer,	/* IN: Buffer to dump	*/
	size_t		size	/* IN: Buffer length	*/
) {
	unsigned char	lv_format[sizeof(KERN_DEBUG) + (43 * DUMP_LINES) + 1] ; /* 43 = line size */
	unsigned char *	lp_cur = lv_format ;
	int 		lc_1 ;
	int		lc_2 ;
	int		lv_max = (size > DUMP_LINES * 10 ? 
					DUMP_LINES * 10 : 
					size) ;
  
	lv_format[0] = '\0' ;
	lp_cur += sprintf(lp_cur, "%s:", KERN_DEBUG);

	/* Format the buffer */
	for (lc_1=0 ; lc_1 < lv_max ; lc_1 += 10) {
		
		/* Print hexa */
		for (
			lc_2 = lc_1 ; 
			((lc_2 < lv_max) && (lc_2 < lc_1 + 10)) ;
			lc_2++
		) {
			sprintf(lp_cur, "%02x ", buffer[lc_2]) ;
			lp_cur 	+= 3 ; /* wrote 3 chars */
		}
		/* Print Padding */
		for (;lc_2 < lc_1 + 10 ; lc_2++) {
			sprintf(lp_cur, "   ");
			lp_cur 	+= 3 ; /* wrote 3 chars */
		}

		/* Print separator */
		sprintf(lp_cur, "- ") ;
		lp_cur += 2 ;

		/* Print ASCII */
		for (
				lc_2 = lc_1;
				(lc_2 < lv_max) && (lc_2 < lc_1 + 10) ;
				lc_2++
		) {
			sprintf(
				lp_cur++, 
				"%c",
				((buffer[lc_2]>= ' ' && buffer[lc_2]<0x7F) ?
					buffer[lc_2] :
					'.')	
			);
		}
		/* Add EOL */
		sprintf(lp_cur++, "\n") ;
	}

	/* Print Dump */
	printk(
		KERN_DEBUG "D ECI_USB [%s:%d] : %s [%d] %s\n%s\n",
		file,
		line,
		txt,
		size,
		(lv_max != size ? "(truncated)" : ""),
		lv_format) ;
}

		
#else

/* DBG_OUT absorb */
#define DBG_OUT(X...)

/* DBG_RAW_OUT absorb */
#define DBG_RAW_OUT(X...)

#endif

/**********************************************************************
                    Constants
***********************************************************************/

#define ECI_WAN_VENDOR_ID 0x0915
#define ECI_WAN_PRODUCT_ID 0x8000

#define ECI_INT_EP  0x86
#define ECI_ISO_PIPE 0x88
#define ECI_BULK_PIPE 0x02

#define ECI_NB_ISO_PACKET 10
#define ECI_ISO_PACKET_SIZE 448
#define ECI_ISO_BUFFER_SIZE (ECI_ISO_PACKET_SIZE * ECI_NB_ISO_PACKET)

/**********************************************************************
                    Globals	
***********************************************************************/

#define ERR_OUT(fmt, argz...) \
	printk ( \
		KERN_ERR "E ECI_USB : " fmt, \
		##argz \
	)

#define WRN_OUT(fmt, argz...) \
	printk ( \
		KERN_WARNING "W ECI_USB : " fmt, \
		##argz \
	)

static const char eci_drv_name[] =  "ECI HiFocus ADSL Modem";
static const char eci_drv_label[] =  "ECI";
static const unsigned char eci_init_setup_packets[] =
{
#include "usb_packets.h"
};


/**********************************************************************
                    USB stuff
***********************************************************************/

static const struct usb_device_id eci_usb_deviceids[] =
{
	{ USB_DEVICE ( ECI_WAN_VENDOR_ID , ECI_WAN_PRODUCT_ID ) } ,
	{ } 
};

static void eci_init_vendor_callback(struct urb *urb);
static void eci_int_callback(struct urb *urb);
static void eci_iso_callback(struct urb *urb);
static void _eci_send_init_urb(struct urb *eciurb);
static void eci_bulk_callback(struct urb *urb);




/**********************************************************************
                    ATM stuff
***********************************************************************/

#ifdef __USE_ATM__

/* -- MACROZ -- */

/* 
 * Retrieve private data from vcc
 */
#define GET_PRIV(atmdev) ((struct eci_instance *) ((atmdev)->dev_data))

/*
 * Free a given skb
 */
#define FREE_SKB(vcc, skb) \
	do { \
		if ((vcc)->pop)	(vcc)->pop((vcc), (skb)) ; \
		else 		dev_kfree_skb((skb)) ; \
	} while (0)

/* -- Constants -- */

/* Number max of UNI cells needed to transport an AAL5 frame */
#define ATM_MAX_CELLS		(unsigned int)	1366


/* -- Types -- */
struct eci_instance ; /* forward declaration */


/*----------------------------------------------------------------------
		P R I V A T E   D E C L A R A T I O N S
  ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/*--------------------------- G E N E R A L ----------------------------*/

/*-- M A C R O S -------------------------------------------------------*/
/*-- T Y P E S ---------------------------------------------------------*/

/* Boolean */
#if defined(true) & defined(false)
typedef enum {mytrue = true, myfalse = false } bool ;
#else
typedef enum {true = 1, false = 0 } bool ;
#endif /* true & false */

/* Bytes */
typedef unsigned char byte ;

/*-- C O N S T A N T S -------------------------------------------------*/
/*-- V A R I A B L E S -------------------------------------------------*/
/*-- F U N C T I O N S -------------------------------------------------*/

/*----------------------------------------------------------------------*/

/*-------------------------- U N I  C E L L S --------------------------*/

/*-- M A C R O S -------------------------------------------------------*/

#define ATM_CELL_HDR		(size_t)5	/* Cell Header lenght	*/
#define ATM_CELL_PAYLOAD	(size_t)48	/* Cell Payload length	*/

/*-- T Y P E S ---------------------------------------------------------*/

struct uni_cell ;				/* forward declaration	*/
typedef struct uni_cell uni_cell_t ;

struct uni_cell {
	/*
	 * Do not insert any thing before
	 * To keep &uni_cell = &raw
	 */
	byte 		raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD] ;

	/* -- for queue managements -- */
	uni_cell_t * 	next ;
} ;



/*-- C O N S T A N T S -------------------------------------------------*/
/*-- V A R I A B L E S -------------------------------------------------*/
/*-- F U N C T I O N S -------------------------------------------------*/

/* Alloc a new UNI Cell */
static uni_cell_t * _uni_cell_alloc(void) ;

/* Free an UNI Cell 	*/
static void _uni_cell_free(uni_cell_t *) ;

/* Format a cell */
static int _uni_cell_format(
	int,			/* IN: VPI			*/
	int,			/* IN: VCI			*/
	bool,			/* IN: Flag for last cell	*/
	size_t,			/* IN: Payload size		*/
	byte *,			/* IN: Payload data		*/
	uni_cell_t *		/* OUT: Formated UNI cell	*/
) ;

/* 
 * Get VPI from cell (-1 if error) 
 */
static int _uni_cell_getVpi(
	uni_cell_t *		/* IN: the cell			*/
) ;

/* Get VCI from cell (-1 if error) */
static int _uni_cell_getVci(
	uni_cell_t *		/* IN: the cell			*/
) ;

/* check if this is the last cell (true on error) */
static bool _uni_cell_islast(
	uni_cell_t *		/* in: the cell			*/
) ;

/* Get The Payload */
static byte * _uni_cell_getPayload(
	uni_cell_t *		/* in: the cell			*/
) ;

/*----------------------------------------------------------------------*/

/*--------------------- U N I  C E L L  Q U E U E ----------------------*/

/*-- M A C R O S -------------------------------------------------------*/
/*-- T Y P E S ---------------------------------------------------------*/

struct uni_cell_q {
	unsigned int		nbcells ;	/* Number of cells	*/
	uni_cell_t *		first ;		/* First list item	*/
	uni_cell_t *		last ;		/* Last list item	*/
} ;

typedef struct uni_cell_q uni_cell_q_t ;

/*-- C O N S T A N T S -------------------------------------------------*/
/*-- V A R I A B L E S -------------------------------------------------*/

static uni_cell_t *		gp_ucells	= NULL ;

static spinlock_t		gl_ucells_lock 	= SPIN_LOCK_UNLOCKED ;

/*-- F U N C T I O N S -------------------------------------------------*/

/* Init a queue */
static int _uni_cell_qinit(uni_cell_q_t *) ;

/* Alloc a new queue */
static uni_cell_q_t * _uni_cell_qalloc(void) ;

/* Free a queue */
static void _uni_cell_qfree(uni_cell_q_t *) ;

/* Check if the queue is empty */
static bool _uni_cell_qisempty(uni_cell_q_t *) ; 

/* Push new UNI Cell in Queue */
static int _uni_cell_qpush(
	uni_cell_t *,		/* IN: Cell to insert	*/
	uni_cell_q_t *		/* IN: Target queue	*/
) ;

/* Pop an UNI Cell from Queue */
static uni_cell_t * _uni_cell_qpop(uni_cell_q_t *) ;

/* Get UNI Cell on head of Queue */
static uni_cell_t * _uni_cell_qhead(uni_cell_q_t *) ;

/*----------------------------------------------------------------------*/

/*------------------------ A A L 5  F R A M E --------------------------*/

/*-- M A C R O S -------------------------------------------------------*/
/*-- T Y P E S ---------------------------------------------------------*/

struct aal5 ;					/* forward declaration	*/
typedef struct aal5 aal5_t ;

struct aal5 {
	bool		is_complete ;
	bool		is_valid ;
	int		vci ;
	int		vpi ;

	size_t		nbcells ;

	uni_cell_t *	pccell ;
	uni_cell_t *	plastcell ;
	
	uni_cell_t *	pthecells ;

	/* -- For built purpose -- */
	unsigned long	_crc ;
} ;

/*-- C O N S T A N T S -------------------------------------------------*/
/*-- V A R I A B L E S -------------------------------------------------*/
/*-- F U N C T I O N S -------------------------------------------------*/

/* Init a new AAL5 Frame */
static int _aal5_init(aal5_t *) ;
	
/* Alloc a new AAL5 frame */
static aal5_t * _aal5_alloc(void) ;

/* Free an AAL5 frame */
static void _aal5_free(aal5_t *) ;

/* Reset an AAL5 frame */
static void _aal5_clean(aal5_t *) ;

/* Get VPI from AAL5 */
static int _aal5_getVpi(
	aal5_t *		/* IN: the frame		*/
) ;

/* Get VCI from AAL5 */
static int _aal5_getVci(
	aal5_t *		/* IN: the frame		*/
) ;

/* check if this is the frame is completed */
static bool _aal5_iscomplete(
	aal5_t *		/* IN: the frame		*/
) ;

/* check if this is the frame is valid */
static bool _aal5_isvalid(
	aal5_t *		/* IN: the frame		*/
) ;

/* Append a new cell */
static int _aal5_add_cell(
	aal5_t *,		/* IN: the frame		*/
	uni_cell_t *		/* IN: the new cell		*/
) ;

/* Get Next Cell of the AAL5 frame */
static uni_cell_t * _aal5_get_next(aal5_t *) ;


/* -- Interface -- */
static int eci_atm_open(struct atm_vcc *vcc,short vpi, int vci);
static void eci_atm_close(struct atm_vcc *vcc);
static int eci_atm_ioctl(struct atm_dev *vcc,unsigned int cmd, void *arg);
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb);

/* -- Private Functions -- */
static u32 _calc_crc(unsigned char *, int, unsigned int) ;
static int _eci_tx_aal5( 
		int, int, 
		struct eci_instance *, struct sk_buff *
) ;
static void _eci_bulk_callback(struct urb *) ;

/* Interface ATM/USB */
static int _eci_usb_send_urb(
	struct eci_instance*,	/* IN: ref to mod info	*/
	aal5_t*			/* IN: ref to AAL5 to send */
) ;

/*
 * Send data from USB (single uni cell) to ATM
 */
static int _eci_usb_rx(
	uni_cell_t *		/* IN: UNI Cell		*/
) ;

static const struct atmdev_ops eci_atm_devops =
{
	open: 	eci_atm_open,
	send: 	eci_atm_send,
	close: 	eci_atm_close,
	ioctl:	eci_atm_ioctl
};
#endif /* __USE_ATM__ */

/**********************************************************************	
                    Module Stuff
***********************************************************************/

#ifdef MODULE
MODULE_DEVICE_TABLE ( usb , eci_usb_deviceids ) ;

/*	prototypes	*/
static int __init eci_init(void);
static void __exit eci_cleanup(void);

MODULE_AUTHOR( "Eric Bardes, Jean-Sebastien Valette" );
MODULE_DESCRIPTION( "ECI HiFocus ADSL Modem Driver");
module_init (eci_init);
module_exit (eci_cleanup);


#endif
/**********************************************************************
                    USB Driver stuff
***********************************************************************/

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
	int 			minor;
	wait_queue_head_t 	eci_wait;	/*	no more used	*/
	const unsigned char	*setup_packets; /* 	For init	*/
	struct urb		*isourbs;	/*	For Iso transfer */
	struct urb		*vendor_urb;	/*	For init and Answer 
							to INT		*/
	struct urb		*interrupt_urb;	/*	INT pooling	*/
	unsigned char 		*interrupt_buffer;
	unsigned char 		*pooling_buffer;


	/* -- Transmission Elements -- */
	spinlock_t		rx_q_lock ;
	uni_cell_q_t *		prx_q ;
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


/**********************************************************************
                   Driver initialisation 

***********************************************************************/


static int __init eci_init(void)
{	
	int lv_res = 0 ;

	/*	Must register driver and claim for hardware */
	DBG_OUT("init module in\n");

	printk("ECI HiFocus ADSL Modem Driver loading\n");
	lv_res = usb_register(&eci_usb_driver) ;

	DBG_OUT("init module out\n");

	return lv_res ;

}


static void __exit eci_cleanup(void)
{
	DBG_OUT("cleanup in\n");
	usb_deregister(&eci_usb_driver);

	/* Free Unused UNI Cell List */
	{
		uni_cell_t * lp_cell = gp_ucells ;
		while (lp_cell) {
			gp_ucells = lp_cell->next ;
			kfree(lp_cell) ;
			lp_cell = gp_ucells ;
		}
	}

	DBG_OUT("cleanup out\n");
	return;
};
	
void *eci_usb_probe(struct usb_device *dev,unsigned int ifnum , 
	const struct usb_device_id *id)
{
	struct eci_instance *out_instance= NULL;
	struct urb *eciurb, *tmpurb;
	/* unused : int dir; */
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
		out_instance = kmalloc(sizeof(struct eci_instance), 
							GFP_KERNEL);
		if(out_instance)
		{
			MOD_INC_USE_COUNT;
			eci_instances=out_instance; 
			out_instance->usb_wan_dev= dev;
			out_instance->setup_packets = eci_init_setup_packets;
			/*init_waitqueue_head(&out_instance->eci_wait);*/
		}
		else
		{
			ERR_OUT("out of memory\n");
			return 0;
		}
		DBG_OUT("Setting configuration 1\n");
		if(usb_set_configuration(dev,1)<0)
		{
			ERR_OUT("Can't set interface\n");
			return 0;
		}
		DBG_OUT("Setting interface\n");
		if(usb_set_interface(dev,0,4)<0)
		{
			ERR_OUT("Cant set configuration\n");
			return 0;
		}
		/*
			send 20 Urbs to the iso EP
		*/
		DBG_OUT("Init ISO urbs\n");
		out_instance->isourbs = 0;
		for(eci_isourbcnt=0;eci_isourbcnt<20;eci_isourbcnt++)
		{
			if(!(eciurb=usb_alloc_urb(ECI_NB_ISO_PACKET)))
			{
				ERR_OUT(
					"not enought memory for iso urb %d\n",
 					eci_isourbcnt);
				return 0;
			}
			if(!(eciurb->transfer_buffer=kmalloc(
					ECI_ISO_BUFFER_SIZE, GFP_KERNEL)))
			{
				ERR_OUT(
					"not enought memory for iso buffer %d\n",
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
				eciurb->iso_frame_desc[i].actual_length = 0 ;
			}
		}
		tmpurb = out_instance->isourbs;
		while(tmpurb->next) tmpurb=tmpurb->next;
		tmpurb->next = out_instance->isourbs;
		if(usb_submit_urb(eciurb))
		{
			ERR_OUT("error couldn't send iso urbs\n");
			return 0;
		}
		/*
			Should reset the Iso EP and send 20 Urbs
		*/
		DBG_OUT("Init INT endpoint\n");
		if(!(eciurb=usb_alloc_urb(0)))
		{
			ERR_OUT("Can't allocate int urb !\n");
			return 0;
		}
	/*	interval tacken from cat /proc/bus/usb/devices values	*/
		out_instance->interrupt_urb = eciurb;
		if(!(out_instance->interrupt_buffer=kmalloc(64,GFP_KERNEL)))
		{
			ERR_OUT("Can't allocate int buffer\n");
			return 0;
		}
		if(!(out_instance->pooling_buffer = kmalloc(34+8, GFP_KERNEL)))
		{
			ERR_OUT("Can't alloc buffer for interrupt polling\n");
			return 0;
		}
		eciurb->setup_packet = eciurb->transfer_buffer  +34;	
		DBG_OUT("interrupt buffer = %p\n", out_instance->interrupt_buffer);
		FILL_INT_URB(eciurb, dev, usb_rcvintpipe(dev, ECI_INT_EP), 
			out_instance->interrupt_buffer,64,
			eci_int_callback,out_instance,3);
		if(usb_submit_urb(eciurb))
		{
			ERR_OUT("error couldn't send interrupt urb\n");
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
		eciurb=usb_alloc_urb(0);
		if(!eciurb)
		{
			ERR_OUT("Can't allocate urb\n");
			return 0;
		};
		memset(eciurb, 0, sizeof(struct urb));
		/*
			Next allocate buffer for data transmition.
			Last 8 byte will be reserved for setupackets
		*/
		if(!(eciurb->transfer_buffer = kmalloc((64*1024), GFP_KERNEL)))
		{
			ERR_OUT(" out of memory\n");
			return 0;
		};
		/*
			Initialise all urb struct
		*/
		eciurb->context = out_instance;
		out_instance->vendor_urb = eciurb;
		_eci_send_init_urb(eciurb);
	
	}	
	DBG_OUT("Probe: done with usb\n");			
	
#ifdef __USE_ATM__

	DBG_OUT(" doing ATM\n");

	/*
	 * Init Queues
	 */
	out_instance->prx_q = _uni_cell_qalloc() ;
	if (!out_instance->prx_q) {
		ERR_OUT("Failed allocating RX Q\n") ;
		return 0 ;
	}

	out_instance->atm_dev = atm_dev_register(
			eci_drv_label,
			&eci_atm_devops,
			-1,
			NULL) ;
	if(out_instance->atm_dev)
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
		ERR_OUT("Can't init ATM device\n");
	}
#endif /* __USE_ATM__ */
	
	DBG_OUT("out_instance : %p\n", out_instance);
	
	DBG_OUT("Probe out\n");
	return out_instance;
};

void eci_usb_disconnect(struct usb_device *dev, void *p)
{
	struct urb *urb;
	
	DBG_OUT("disconnect in\n");
	if(eci_instances)
	{
#ifdef __USE_ATM__
		DBG_OUT("disconnect : freeing atm_dev\n");
		atm_dev_deregister(eci_instances->atm_dev);

		/*
		 * Free the IO Q
		 */
		_uni_cell_qfree(eci_instances->prx_q) ;

#endif /* __USE_ATM__ */

		DBG_OUT("disconnect : freeing int urb\n");
		usb_unlink_urb(eci_instances->interrupt_urb);
		usb_free_urb(eci_instances->interrupt_urb);
		DBG_OUT("disconnect : freeing iso urbs\n");
		urb=eci_instances->isourbs;
		while(urb->next!=eci_instances->isourbs) urb=urb->next;
		urb->next = 0;
		while((urb = eci_instances->isourbs) != NULL)
		{
			DBG_OUT("Freeing one iso\n");
			eci_instances->isourbs = urb->next;
			usb_unlink_urb(urb);
			usb_free_urb(urb);
		}
		DBG_OUT("disconnect : freeing instance %p\n", eci_instances);
		kfree(eci_instances);
		eci_instances=NULL;
	}
	MOD_DEC_USE_COUNT;
	DBG_OUT("disconnect out\n");

};



int eci_usb_ioctl(struct usb_device *usb_dev,unsigned int code, void * buf)
{
	DBG_OUT("eci_usb_ioctl in\n");
	DBG_OUT("eci_usb_ioctl out\n");
	return -1;
};

/*******************************************************************************
		
		ATM functions

*******************************************************************************/
#ifdef __USE_ATM__

/*----------------------------------------------------------------------*/
static int eci_atm_open(struct atm_vcc *vcc, short vpi, int vci)
{
	int lv_rc ;
	
	DBG_OUT("eci_atm_open in [%hd.%d]\n", vpi, vci);

#if defined(ATM_VPI_UNSPEC) && defined(ATM_VCI_UNSPEC)
	/* Check that VCI & VPI are specified */
	if (vpi == ATM_VPI_UNSPEC || vci == ATM_VCI_UNSPEC) {
    		WRN_OUT("rejecting open with unspecified VPI/VCI\n");
		return -EINVAL;
	}
#endif /* ATM_VPI_UNSPEC && ATM_VCI_UNSPEC */
	
	/*
	 * Check that a context don't exists already
	 */
	lv_rc = atm_find_ci(
			vcc,
			&vpi,
			&vci) ;
	if (lv_rc != 0) {
		ERR_OUT("atm_find_ci failed [%d]", lv_rc) ;
		return lv_rc ;
	}
	DBG_OUT("params after find : [%hd.%d]\n", vpi, vci) ;

	set_bit(ATM_VF_ADDR,&vcc->flags);

	vcc->vpi = vpi ;
	vcc->vci = vci ;

	clear_bit(ATM_VF_READY,&vcc->flags); /* just in case ... */

	/* Check QOS type */
	if (vcc->qos.aal != ATM_AAL5) {
		WRN_OUT("not implemented frame type [%d]\n", vcc->qos.aal) ;
		return -EINVAL ;
	}

	/* Indicate we're done! */
	set_bit(ATM_VF_READY, &vcc->flags);

	DBG_OUT("eci_atm_open out\n");

	return 0;
};

/*----------------------------------------------------------------------*/
static void eci_atm_close(struct atm_vcc *vcc)
{
	DBG_OUT("eci_atm_close in\n");
	DBG_OUT("eci_atm_close out\n");
};

/*----------------------------------------------------------------------*/
static int eci_atm_ioctl(struct atm_dev *vcc,unsigned int cmd, void *arg)
{
	DBG_OUT("eci_atm_ioctl in\n");
	DBG_OUT("eci_atm_ioctl out\n");
	return -1;
};

/*----------------------------------------------------------------------*/
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb)
{
	struct eci_instance *	lp_outinst	= NULL ;
	int			lv_rc		= 0 ;
	
	DBG_OUT("eci_atm_send in\n");

	/* Check interface */
	if (!skb) {
		ERR_OUT("Invalid Socket Buffer\n") ;
		return -EINVAL;
	}

	if (!vcc) {
		ERR_OUT("Invalid VCC\n") ;
		/* Free the socket buffer */
		dev_kfree_skb(skb) ;
		return -EINVAL ;
	}
		
	/* Get private data */
	lp_outinst = GET_PRIV(vcc->dev) ;

	if (!lp_outinst) {
		FREE_SKB(vcc, skb) ;
		atomic_inc(&vcc->stats->tx_err) ;
		ERR_OUT("no eci data provided\n") ;
		return -EINVAL;
	}
	
	/* Manage Send */
	switch (vcc->qos.aal) {
	case ATM_AAL5:
		DBG_OUT("Manage new AAL5 data\n") ;
		lv_rc = _eci_tx_aal5(vcc->vpi, vcc->vci, lp_outinst, skb) ;
		if (lv_rc) {
			ERR_OUT("_eci_tx_aal5 failed\n") ;
			atomic_inc(&vcc->stats->tx_err) ;
		} else {
			atomic_inc(&vcc->stats->tx) ;
			DBG_OUT("AAL5 sent\n") ;
		}
		break ;

	case ATM_AAL0:
		DBG_OUT("MANAGE new AAL0 data\n") ;
		if (skb->len != ATM_CELL_SIZE-1) {
			ERR_OUT("Socket buffer invalid size\n") ;
			lv_rc = -EINVAL ;
		} else {
			WRN_OUT("AAL0 NOT IMPLEMENTED\n") ;
			lv_rc = -ENOTSUPP ;
		}
		atomic_inc(&vcc->stats->tx_err) ;
		break ;
	}

	/* Free Socket Buffer */
	FREE_SKB(vcc, skb) ;

	DBG_OUT("eci_atm_send out\n");
	return lv_rc ;
};

/*----------------------------------------------------------------------*/

/* -- Private Functions ------------------------------------------------*/
/*----------------------------------------------------------------------*/


#endif /* __USE_ATM__ */

/*******************************************************************************


		
		USB functions






*******************************************************************************/

static void _eci_send_init_urb(struct urb *eciurb)
{
	struct eci_instance *instance;
	unsigned char *setuppacket;
	unsigned int pipe;
	int size;

	
	instance = eciurb->context;
	if(instance->setup_packets[0])
	{
		setuppacket = eciurb->transfer_buffer +  64 * 1024 -8;
		memcpy(setuppacket, instance->setup_packets,8);
		size = (instance->setup_packets[7] << 8 ) |
				instance->setup_packets[6];
		DBG_RAW_OUT("Setup Packet", setuppacket, 8) ;
			/*
				If write URB then read the buffer and
				set endpoint else just set enpoint
			*/
		if(setuppacket[0] & 0x80)
		{
			pipe = usb_rcvctrlpipe(instance->usb_wan_dev,0);
			DBG_OUT("Read URB\n");
		}
		else
		{
			memcpy(eciurb->transfer_buffer, 
				instance->setup_packets+8,size);
			DBG_OUT("Write URB\n");
			DBG_RAW_OUT("Buffer : ", eciurb->transfer_buffer, size);
			pipe = usb_sndctrlpipe(instance->usb_wan_dev,0); 
		}
		FILL_CONTROL_URB(eciurb, instance->usb_wan_dev, pipe, 
			setuppacket, eciurb->transfer_buffer, 
			size, eci_init_vendor_callback,
			instance);
		DBG_OUT("Sending URB\n");
		if(usb_submit_urb(eciurb))
		{
			ERR_OUT("error couldn't send init urb\n");
			return;
		}
		DBG_OUT("URB Sent \n");
		instance->setup_packets += 
			(eciurb->setup_packet[0] & 0x80) ? 8 : 8 + size;
	}
	else
	{	/*	gotta free urb and buffer !	*/
		kfree(eciurb->transfer_buffer);
		eciurb->transfer_buffer = 0;
	}
}

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
	/* unused: struct usb_device	*dev; */
	int size;
	/* unused: int i; */
	/* unused: unsigned char *buffer; */

	DBG_OUT("init callback called !\n");
	instance = (struct eci_instance *) urb->context;
/*	dev = instance->usb_wan_dev;
	DBG_OUT("dev = %p\n", dev);	unused */

/*
	If urb status is set warn about it, else do what we gotta do
*/

	if(urb->status)
		ERR_OUT("Urb with status %d\n" , urb->status);
	else
	{
		/*	Check For datarecived and wanted data length */
		if( urb->actual_length != urb->transfer_buffer_length )
			WRN_OUT(
				"expecting %d, got %d\n",
				urb->transfer_buffer_length, 
				urb->actual_length) ;

#ifdef DEBUG
		if ((urb->setup_packet[0] & 0x80))
		{
			size = ((instance->setup_packets[7]<<8) | 
					instance->setup_packets[6]);
			DBG_RAW_OUT(
					"Buffer:",
					(char *) urb->transfer_buffer,
					size) ;
		}
#endif	/*	DEBUG	*/
		if ( !((urb->setup_packet[0] & 0x40) == 0x40 && urb->setup_packet[1]== 0xdc
			&&((urb->setup_packet[3] * 256 + urb->setup_packet[2])== 0x00) && 
			((urb->setup_packet[5] * 25 + urb->setup_packet[4]) == 0x00)))
		{
			/*	
				sould send next VENDOR URB
			*/
			DBG_OUT("Calling _eci_send_init_urb from Vendor \n");
			_eci_send_init_urb(instance->vendor_urb);
		}
#ifdef DEBUG
		else
			DBG_OUT("Waiting for data on 0x86 endpoint\n");
#endif /* DEBUG */
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
static void eci_int_callback(struct urb *urb)
{
	struct eci_instance *instance;
	struct urb *eci_vendor;
	int i; 		/*loop counter	*/
	int outi = 0; 
	unsigned char *in_buf, b1, b2;
	static int eci_int_count = 0;
	static unsigned char replace_b1[] = { 0x73, 0x73, 0x63, 0x63,
		0x63, 0x63, 0x73, 0x73, 0x63, 0x63, 0x63 , 0x63 };
	static unsigned char replace_b2[] = { 0x11, 0x11, 0x13, 0x13,
		0x13, 0x13, 0x11, 0x11, 0x1, 0x1, 0x1, 0x01 };
	static unsigned char eci_outbuf[34] = {
		0xff, 0xff, 
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc
	}; 


	/*	DBG_OUT("Int callBack in\n"); Too much debug info	*/
	instance = (struct eci_instance *) urb->context;
	if(urb->actual_length)
	{
		if(urb->actual_length!=64)
		{	/*		Synchronisation 
					Check Data returned by int urb
					and send Next Vendor
			*/
			DBG_OUT(
				"EP INT received %d bytes\n",
				urb->actual_length);
			DBG_OUT("Calling _eci_send_init_urb from Int \n");
			_eci_send_init_urb(instance->vendor_urb);
		}
		else
		{
/*	
	
	Already synchronized just answer	

	Here what we do :
		Send the received buffer exept for 0x7311 which is
		replaced by 0x6313, 0x6301, 0x6313 , 0x6353 ...
		Thanxs benoit papillault for the algo. 

*/
			in_buf = instance->interrupt_buffer;
			outi = 0;
			for(i=0; i<34;) 
				instance->pooling_buffer[i] = 
					eci_outbuf[i++];
			for(i=0;i<15;i++)
			{
				b1 = in_buf[6+2*i+0];
				b2 = in_buf[6+2*i+1];
				if( b1 != 0x0c || b2 !=0x0c)
				{

					if((b1 == 0x73) && (b2 == 0x11)) 
					{
					  eci_int_count %= 12;
					  b1 = replace_b1[eci_int_count];
					  b2 = replace_b2[eci_int_count++];
					  if(!eci_int_count)
					  {
						replace_b2[8] =
						replace_b2[9] =
						replace_b2[10] =
						replace_b2[11] = 0x53;
					  }
					}
					instance->pooling_buffer[10 + 
						outi++] = b1;
					instance->pooling_buffer[10 + 
						outi++] = b2;
				}
			}
			if(outi)
			{
				usb_control_msg(instance->usb_wan_dev, 
					usb_pipecontrol(0), 0xdd, 0x40, 0xc02,
				 	0x580 , eci_outbuf, 
					sizeof(eci_outbuf), HZ);
			}
			DBG_OUT("EP INT received 64 bytes\n");
			DBG_RAW_OUT("EP INT datas :",in_buf, 64); 
		}
	}
	DBG_OUT("Int callBack out\n");
}

static void eci_iso_callback(struct urb *urb)
{
	struct eci_instance *instance;
	int i;

	if (!urb->status) {
		for (i=0;i<ECI_NB_ISO_PACKET;i++) {
			if (!urb->iso_frame_desc[i].status &&
					urb->iso_frame_desc[i].actual_length) {
				DBG_OUT("Data available in frame [%d]\n", i) ;
				DBG_RAW_OUT(
					"received data", 
					urb->transfer_buffer + urb->iso_frame_desc[i].offset,
					urb->iso_frame_desc[i].actual_length) ;
			}
		}
	}
					
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
		urb->iso_frame_desc[i].actual_length = 0 ;
	}
	/*
	DBG_OUT("Iso Callback Exit\n");
	*/
}

static int eci_usb_send_urb(struct eci_instance *instance, struct aal5 *aal5)
{
	int 		ret;		/*	counter			*/
	int 		nbcell;		/*	cellcount expected	*/
	unsigned	char *buf;	/*	urb buffer		*/
	int		buflen;		/*	urb buffer len		*/
	int		bufpos;		/*	position in urb buffer	*/
	struct uni_cell	*cell;		/*	current computed cell	*/
	struct urb	*urb;		/*	urb pointer to sent urb	*/

	nbcell = aal5->nbcells;	/*	Must be change to use	*/
	buflen = ATM_CELL_PAYLOAD * 
			aal5->nbcells;	/*	abstraction function	*/
	if(!(buf=(unsigned char *)kmalloc(buflen, GFP_KERNEL)))
	{
		ERR_OUT("Can't allocate bulk urb Buffer\n");
		return(0);
	}
	bufpos = ret = 0;
	while((cell = _aal5_get_next(aal5)))
	{
		if( memcpy(buf + bufpos, cell->raw, ATM_CELL_PAYLOAD) ==
			ATM_CELL_PAYLOAD)
		{
			bufpos += ATM_CELL_PAYLOAD;
			ret++;
		}
		else
			break;
	}
	if( ret != nbcell)
	{
		ERR_OUT("Error computing bulk buffer\n");
		ERR_OUT("Expecting %d cell got %d\n", nbcell, ret);
	}
	if(!(urb = usb_alloc_urb(0)))
	{
		ERR_OUT("Can't alloacate bulk URB\n");
		return(0);
	}
	FILL_BULK_URB(urb, instance->usb_wan_dev, 
		usb_sndbulkpipe(instance->usb_wan_dev, ECI_BULK_PIPE),
		buf, bufpos, eci_bulk_callback, instance);
	if(!(usb_submit_urb(urb)))
	{
		ERR_OUT("Can't submit bulk urb\n");
		return(0);
	}
	_aal5_free(aal5);
	return(ret);	
}

static void eci_bulk_callback(struct urb *urb)
{
	if(urb->status)
	{
		ERR_OUT("Error on Bulk URB, status %d\n", urb->status);
	}
	kfree(urb->transfer_buffer);
}
/**********************************************************************
		END USB CODE
*************************************************************************/
static int _eci_make_aal5(
		aal5_t * 		paal5,
		int			vpi,
		int			vci,
		bool			islast,
		size_t			szdata,
		byte *			pdata
) {
	uni_cell_t *	lp_cell	= NULL ;
	int		lv_rc	= 0 ;

	DBG_OUT("_eci_make_aal5 in\n") ;

	/* Check Interface */
	if (!paal5 || !szdata || !pdata) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	/* Alloc new cell */
	lp_cell = _uni_cell_alloc() ;
	if (!lp_cell) {
		ERR_OUT("Not enought memory\n") ;
		return -ENOMEM ;
	}
		
	/* Format it */
	lv_rc = _uni_cell_format(
			vpi,
			vci,
			islast,
			szdata,
			pdata,
			lp_cell) ;
	if (lv_rc) {
		ERR_OUT("_uni_cell_format failed\n") ;
		_uni_cell_free(lp_cell) ;
		return lv_rc ;
	}

	/* Add new cell to AAL5 frame */
	lv_rc = _aal5_add_cell(
			paal5,
			lp_cell) ;
	if (lv_rc) {
		ERR_OUT("_aal5_add_cell failed\n") ;
		_uni_cell_free(lp_cell) ;
		return lv_rc ;
	}

	DBG_OUT("_eci_make_aal5 out\n") ;
	return 0 ;

}
/*----------------------------------------------------------------------
 *
 * Complete the AAL5 frame received from ATM layer
 * and split it into several UNI cells
 * We need to format the AAL5 trailer
 * Then enqueue each cells
 *
 */
static int _eci_tx_aal5(
		int			vpi,	/* IN: VPI		*/
		int			vci,	/* IN: VCI		*/
		struct eci_instance *	pinstance,
		struct sk_buff *	pskb	
) {
	size_t		lv_szdata	= 0 ;
	byte *		lp_data		= NULL ;
	byte		lv_aal5trl[ATM_CELL_PAYLOAD] ;
	u32		lv_crc32	= 0 ;
	int		lc_1 		= 0 ;
	int		lv_rc 		= 0 ;
	aal5_t *	lp_aal5 	= NULL ;
	
	/* Check Interface */
	if (!pinstance || !pskb || !pskb->len || !pskb->data) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	lv_szdata 	= pskb->len ;
	lp_data		= pskb->data ;
	
	if (lv_szdata > ((2^16) - 1)) {
		ERR_OUT("data to long to be hold in aal5 frame\n") ;
		return -EINVAL ;
	}

	lp_aal5 = _aal5_alloc() ;
	if (!lp_aal5) {
		ERR_OUT("Failed to init new AAL5 frame\n") ;
		return -ENOMEM ;
	}

	/*
	 * Format the AAL5 trailer
	 * |------------------|---------|---------|--------|-----|
	 * |    User Data     |   PAD   | Control | Length | CRC |
	 * |------------------|---------|---------|--------|-----|
	 *        0-65535        0-47        2        2       4
	 *
	 * PAD : to make the full frame to fit 48 bytes boundery
	 * Control(UU + CPI) : Reserved (set to 0)
	 *
	 */

	/* Set the padding and reset Control */
	memset(lv_aal5trl, 0, ATM_CELL_PAYLOAD - 6) ;
	
	/* Set the length (no CPI & no UU) */
	lv_aal5trl[ATM_CELL_PAYLOAD - 6] = (lv_szdata & 0xffff) >> 8 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 5] = (lv_szdata & 0x00ff) ;

	/* Set the CRC after compute */
	lv_crc32 = _calc_crc(
				lp_data,
				lv_szdata,
				-1) ;
	lv_crc32 = ~ _calc_crc(
			lv_aal5trl,
			4,
			lv_crc32) ;

	lv_aal5trl[ATM_CELL_PAYLOAD - 4] = lv_crc32 >> 24 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 3] = (lv_crc32 & 0x00ff0000) >> 12 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 2] = (lv_crc32 & 0x0000ff00) >> 8 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 1] = (lv_crc32 & 0x000000ff) ;

	/* Split AAL5 into UNI Cells */
	lc_1 = 0 ;
	while (lv_szdata > ATM_CELL_PAYLOAD) {
		
		/* Enqueue slice */
		lv_rc = _eci_make_aal5(
				lp_aal5,
				vpi,
				vci,
				false,
				ATM_CELL_PAYLOAD,
				lp_data) ;
		if (lv_rc) {
			ERR_OUT("_eci_make_aal5 failed\n") ;
			_aal5_free(lp_aal5) ;
			return lv_rc ;
		}
		lp_data += ATM_CELL_PAYLOAD ;
		lv_szdata -= ATM_CELL_PAYLOAD ;
	}
	
	/* 
	 * Manage Last data padded cell
	 * 2 possilble cases :
	 * - Enought free room to happend trailer
	 * - Not enought => need one more cell
	 */
	if (lv_szdata > (ATM_CELL_PAYLOAD - ATM_AAL5_TRAILER)) {

		/* Enqueue this before last slice */
		lv_rc = _eci_make_aal5(
				lp_aal5,
				vpi,
				vci,
				false,
				lv_szdata,
				lp_data) ;
		if (lv_rc) {
			ERR_OUT("_eci_make_aal5 failed on last cell\n") ;
			_aal5_free(lp_aal5) ;
			return lv_rc ;
		}
	} else {
		/* copy remaining data in trailer cell */
		memcpy(lv_aal5trl, lp_data, lv_szdata) ;
	}

	/* Manage the last cell */
	lv_rc = _eci_make_aal5(
			lp_aal5,
			vpi,
			vci,
			true,
			ATM_CELL_PAYLOAD,
			lv_aal5trl) ;
	if (lv_rc) {
		ERR_OUT("_eci_make_aal5 failed on last cell\n") ;
		_aal5_free(lp_aal5) ;
		return lv_rc ;
	}

	/* lp_aal5 is free by _eci_usb_send_urb */
	return _eci_usb_send_urb(pinstance, lp_aal5) ;
}

/*----------------------------------------------------------------------
 * 
 * Forward the UNI Cell array to USB layer
 * Map each cells into happend bulk buffers
 * Here the Bulk payload is 64 bytes so we pad to FF
 *
 */
static int _eci_usb_send_urb(
	struct eci_instance *	pinstance,	/* IN: ref to mod info	*/
	aal5_t *		paal5		/* IN: AAL5 to send	*/
) {
	int		lv_rc		= 0 ;
	struct urb *	lp_eciurb 	= NULL ;
	byte *		lp_buffer	= NULL ;
	uni_cell_t *	lp_cell		= NULL ;
	const size_t	lc_szcell	= ATM_CELL_HDR + ATM_CELL_PAYLOAD ;		

	DBG_OUT("_eci_usb_send_urb in\n") ;
	
	/* Check Interface */
	if (!pinstance || !paal5 || !_aal5_iscomplete(paal5)) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	/* Send all cells in tx q */
	while ((lp_cell = _aal5_get_next(paal5))) {
		/*
		 * Encapsulate the cells into Bulk data buffer
		 *
		 */
		lp_buffer = (byte*) kmalloc(lc_szcell, GFP_KERNEL) ;
		if (!lp_buffer) {
			ERR_OUT("not enought memory to send the cell\n") ;
			/* we continue to empty the queue */
			continue ;
		}

		memcpy(
			lp_buffer, 
			_uni_cell_getPayload(lp_cell), 
			lc_szcell) ;


		/* Alloc the new URB */
		lp_eciurb = usb_alloc_urb(0) ;
		if (!lp_eciurb) {
			ERR_OUT("Not enought Memory for iso urb") ;
			kfree(lp_buffer) ;
			/* we continue to empty the queue */
			continue ;

		}

		/* Init the New URB */
		FILL_BULK_URB(
				lp_eciurb,
				pinstance->usb_wan_dev,
				usb_sndbulkpipe(
					pinstance->usb_wan_dev, 
					ECI_BULK_PIPE),
				lp_buffer,
				lc_szcell,
				_eci_bulk_callback,
				pinstance
		) ;

		/* Submit the new URB to the USB host */

		lv_rc = usb_submit_urb(lp_eciurb) ;
		if (lv_rc) {
			/*
			 * lv_rc == -6 == -ENXIO
			 * This error means that this urb is already queued
			 * It should be ignored ???
			 */
			ERR_OUT("usb_submit_urb failed [%d]\n", lv_rc) ;
			usb_free_urb(lp_eciurb) ;
			kfree(lp_buffer) ;
			/* we continue to empty the queue */
			continue ;
		}
		DBG_OUT("Bulk URB %p sent\n", lp_eciurb) ;
	}
	
	DBG_OUT("_eci_usb_send_urb out\n") ;

	return 0 ;
}

/*----------------------------------------------------------------------
 * 
 * Send data from USB (single uni cell) to ATM
 *
 */
static int _eci_usb_rx(
	uni_cell_t *thecell		/* IN: UNI Cell		*/
) {
	int	lv_rc	= 0 ;
	DBG_OUT("_eci_usb_rx in\n") ;
	DBG_OUT("_eci_usb_rx out\n") ;
	return lv_rc ;
}

/*----------------------------------------------------------------------
 * 
 * Send data to ATM
 *
 */
static int _eci_rx_aal5(
		struct eci_instance * pinstance
) {
	uni_cell_t *		lp_cellfirst	= NULL ;
	uni_cell_t *		lp_cellprv	= NULL ;
	uni_cell_t *		lp_cell		= NULL ;
	bool			lf_last 	= false ;
	int			lv_vpi ;
	int			lv_vci ;
	int			lv_crc ;
	size_t			lv_size 	= 0 ;
	struct sk_buff *	lp_skb		= NULL ;

	/* Check Interface */
	if (!pinstance) return -EINVAL ;

	/*
	 * Get the Cells that constitute a full AAL5 frame
	 */
	lp_cellfirst		= 
		lp_cell 	= 
		lp_cellprv 	= _uni_cell_qpop(pinstance->prx_q) ;
	if (!lp_cell) {
		ERR_OUT("Empty rx q\n") ;
		return -EINVAL ;
	}
	lv_size += ATM_CELL_PAYLOAD ;
	lv_crc = _calc_crc(
			_uni_cell_getPayload(lp_cell) + ATM_CELL_HDR,
			ATM_CELL_PAYLOAD,
			-1) ;
	
	while (lp_cell && !(lf_last = _uni_cell_islast(lp_cell))) {

		lp_cellprv->next	= lp_cell ;
		lp_cellprv 		= lp_cell ;
		lp_cell 		= _uni_cell_qpop(pinstance->prx_q) ;

		lv_size 		+= ATM_CELL_PAYLOAD ;
		lv_crc = ~ _calc_crc(
				_uni_cell_getPayload(lp_cell) + ATM_CELL_HDR,
				ATM_CELL_PAYLOAD,
				lv_crc) ;
	}

	if (!lp_cell || !lf_last) {
		/* Something goes wrong */
		ERR_OUT("failed to browse the AAL5 frame\n") ;
		lp_cell = lp_cellfirst ;
		while ((lp_cellprv = lp_cell)) {
			lp_cell = lp_cellprv->next ;
			_uni_cell_free(lp_cellprv) ;
		}
		return -EINVAL ;
	}

	/*
	 * Compare CRC
	 */
	
	/* TODO ... */

	return 0 ;

	
	

}

/*----------------------------------------------------------------------
 * 
 * Callback to handle BULK urb completion
 * Free the transfer buffer + the urb itself
 *
 */
static void _eci_bulk_callback(struct urb *purbsent) {
	char * lp_buf = NULL ;

	DBG_OUT("_eci_bulk_callback in \n") ;

	/* Check Interface */
	if (!purbsent) {
		ERR_OUT("Interface Error\n") ;
		return ;
	}
	
	usb_unlink_urb(purbsent) ;

	lp_buf = purbsent->transfer_buffer ;

	/* Free urb */
	usb_free_urb(purbsent) ;

	/* Free transfert buffer */
	if (lp_buf) kfree(lp_buf) ;

	DBG_OUT("_eci_bulk_callback out\n") ;
}

/*
 * Toolz for CRC computing (from Benoit Papillaut)
 */
#define CRC32_REMAINDER CBF43926
#define CRC32_INITIAL 0xffffffff

#define CRC32(c,crc) \
	(crc32tab[((size_t)(crc>>24) ^ (c)) & 0xff] ^ (((crc) << 8)))

static u32 crc32tab[256] = {
	0x00000000L, 0x04C11DB7L, 0x09823B6EL, 0x0D4326D9L,
	0x130476DCL, 0x17C56B6BL, 0x1A864DB2L, 0x1E475005L,
	0x2608EDB8L, 0x22C9F00FL, 0x2F8AD6D6L, 0x2B4BCB61L,
	0x350C9B64L, 0x31CD86D3L, 0x3C8EA00AL, 0x384FBDBDL,
	0x4C11DB70L, 0x48D0C6C7L, 0x4593E01EL, 0x4152FDA9L,
	0x5F15ADACL, 0x5BD4B01BL, 0x569796C2L, 0x52568B75L,
	0x6A1936C8L, 0x6ED82B7FL, 0x639B0DA6L, 0x675A1011L,
	0x791D4014L, 0x7DDC5DA3L, 0x709F7B7AL, 0x745E66CDL,
	0x9823B6E0L, 0x9CE2AB57L, 0x91A18D8EL, 0x95609039L,
	0x8B27C03CL, 0x8FE6DD8BL, 0x82A5FB52L, 0x8664E6E5L,
	0xBE2B5B58L, 0xBAEA46EFL, 0xB7A96036L, 0xB3687D81L,
	0xAD2F2D84L, 0xA9EE3033L, 0xA4AD16EAL, 0xA06C0B5DL,
	0xD4326D90L, 0xD0F37027L, 0xDDB056FEL, 0xD9714B49L,
	0xC7361B4CL, 0xC3F706FBL, 0xCEB42022L, 0xCA753D95L,
	0xF23A8028L, 0xF6FB9D9FL, 0xFBB8BB46L, 0xFF79A6F1L,
	0xE13EF6F4L, 0xE5FFEB43L, 0xE8BCCD9AL, 0xEC7DD02DL,
	0x34867077L, 0x30476DC0L, 0x3D044B19L, 0x39C556AEL,
	0x278206ABL, 0x23431B1CL, 0x2E003DC5L, 0x2AC12072L,
	0x128E9DCFL, 0x164F8078L, 0x1B0CA6A1L, 0x1FCDBB16L,
	0x018AEB13L, 0x054BF6A4L, 0x0808D07DL, 0x0CC9CDCAL,
	0x7897AB07L, 0x7C56B6B0L, 0x71159069L, 0x75D48DDEL,
	0x6B93DDDBL, 0x6F52C06CL, 0x6211E6B5L, 0x66D0FB02L,
	0x5E9F46BFL, 0x5A5E5B08L, 0x571D7DD1L, 0x53DC6066L,
	0x4D9B3063L, 0x495A2DD4L, 0x44190B0DL, 0x40D816BAL,
	0xACA5C697L, 0xA864DB20L, 0xA527FDF9L, 0xA1E6E04EL,
	0xBFA1B04BL, 0xBB60ADFCL, 0xB6238B25L, 0xB2E29692L,
	0x8AAD2B2FL, 0x8E6C3698L, 0x832F1041L, 0x87EE0DF6L,
	0x99A95DF3L, 0x9D684044L, 0x902B669DL, 0x94EA7B2AL,
	0xE0B41DE7L, 0xE4750050L, 0xE9362689L, 0xEDF73B3EL,
	0xF3B06B3BL, 0xF771768CL, 0xFA325055L, 0xFEF34DE2L,
	0xC6BCF05FL, 0xC27DEDE8L, 0xCF3ECB31L, 0xCBFFD686L,
	0xD5B88683L, 0xD1799B34L, 0xDC3ABDEDL, 0xD8FBA05AL,
	0x690CE0EEL, 0x6DCDFD59L, 0x608EDB80L, 0x644FC637L,
	0x7A089632L, 0x7EC98B85L, 0x738AAD5CL, 0x774BB0EBL,
	0x4F040D56L, 0x4BC510E1L, 0x46863638L, 0x42472B8FL,
	0x5C007B8AL, 0x58C1663DL, 0x558240E4L, 0x51435D53L,
	0x251D3B9EL, 0x21DC2629L, 0x2C9F00F0L, 0x285E1D47L,
	0x36194D42L, 0x32D850F5L, 0x3F9B762CL, 0x3B5A6B9BL,
	0x0315D626L, 0x07D4CB91L, 0x0A97ED48L, 0x0E56F0FFL,
	0x1011A0FAL, 0x14D0BD4DL, 0x19939B94L, 0x1D528623L,
	0xF12F560EL, 0xF5EE4BB9L, 0xF8AD6D60L, 0xFC6C70D7L,
	0xE22B20D2L, 0xE6EA3D65L, 0xEBA91BBCL, 0xEF68060BL,
	0xD727BBB6L, 0xD3E6A601L, 0xDEA580D8L, 0xDA649D6FL,
	0xC423CD6AL, 0xC0E2D0DDL, 0xCDA1F604L, 0xC960EBB3L,
	0xBD3E8D7EL, 0xB9FF90C9L, 0xB4BCB610L, 0xB07DABA7L,
	0xAE3AFBA2L, 0xAAFBE615L, 0xA7B8C0CCL, 0xA379DD7BL,
	0x9B3660C6L, 0x9FF77D71L, 0x92B45BA8L, 0x9675461FL,
	0x8832161AL, 0x8CF30BADL, 0x81B02D74L, 0x857130C3L,
	0x5D8A9099L, 0x594B8D2EL, 0x5408ABF7L, 0x50C9B640L,
	0x4E8EE645L, 0x4A4FFBF2L, 0x470CDD2BL, 0x43CDC09CL,
	0x7B827D21L, 0x7F436096L, 0x7200464FL, 0x76C15BF8L,
	0x68860BFDL, 0x6C47164AL, 0x61043093L, 0x65C52D24L,
	0x119B4BE9L, 0x155A565EL, 0x18197087L, 0x1CD86D30L,
	0x029F3D35L, 0x065E2082L, 0x0B1D065BL, 0x0FDC1BECL,
	0x3793A651L, 0x3352BBE6L, 0x3E119D3FL, 0x3AD08088L,
	0x2497D08DL, 0x2056CD3AL, 0x2D15EBE3L, 0x29D4F654L,
	0xC5A92679L, 0xC1683BCEL, 0xCC2B1D17L, 0xC8EA00A0L,
	0xD6AD50A5L, 0xD26C4D12L, 0xDF2F6BCBL, 0xDBEE767CL,
	0xE3A1CBC1L, 0xE760D676L, 0xEA23F0AFL, 0xEEE2ED18L,
	0xF0A5BD1DL, 0xF464A0AAL, 0xF9278673L, 0xFDE69BC4L,
	0x89B8FD09L, 0x8D79E0BEL, 0x803AC667L, 0x84FBDBD0L,
	0x9ABC8BD5L, 0x9E7D9662L, 0x933EB0BBL, 0x97FFAD0CL,
	0xAFB010B1L, 0xAB710D06L, 0xA6322BDFL, 0xA2F33668L,
	0xBCB4666DL, 0xB8757BDAL, 0xB5365D03L, 0xB1F740B4L
};

/*----------------------------------------------------------------------
 *
 * Compute AAL5 frame crc (code from Benoit Papillaut)
 *
 */
static u32 _calc_crc(
		unsigned char *mem,	/* IN: Buffer		*/
		int len, 		/* IN: Buffer length	*/
		unsigned int initial	/* IN: Initial CRC	*/
) {

	u32 crc = initial ;

	for(; len; mem++, len--)
		crc = CRC32(*mem, crc);

	return (crc);
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 			UNI CELL ABSTRACTION
  ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/* 
 * Alloc a new UNI cell : 
 * 	Check if any availlable in unused list else create it
 */
static uni_cell_t * _uni_cell_alloc(void) {
	
	uni_cell_t *lp_cell = NULL ;
	
	/*
	DBG_OUT("_uni_cell_alloc in\n") ;
	*/

	/* Try to Retreive one from unused list */
	/*
	spin_lock(&gl_ucells_lock) ;
	*/

	if ((lp_cell = gp_ucells) != NULL)
		gp_ucells = lp_cell->next ;
	
	/*
	spin_unlock(&gl_ucells_lock) ;
	*/

	if (!lp_cell) {

		/* 
		 * The list might be empty
		 * Allocate a new one
		 */
		DBG_OUT("No availlable cell create new one\n") ;
		lp_cell = (uni_cell_t *) kmalloc(
				sizeof(uni_cell_t),
				GFP_KERNEL) ;
		if (!lp_cell) {
			ERR_OUT("Not enought memory for new cell\n") ;
			return NULL ;
		}
	}

	/*
	DBG_OUT("_uni_cell_alloc out\n") ;
	*/
	return lp_cell ;
}

/*----------------------------------------------------------------------*/

/*
 * Free a UNI cell :
 * 	Insert it in Unused cells list
 */
static void _uni_cell_free(
		uni_cell_t * pcell
) {
	/*
	DBG_OUT("_uni_cell_free in\n") ;
	*/
	if (pcell) {
		/*
		spin_lock(&gl_ucells_lock) ;
		*/
		pcell->next 	= gp_ucells ;
		gp_ucells 	= pcell ;
		/*
		spin_unlock(&gl_ucells_lock) ;
		*/
	}
	/*
	DBG_OUT("_uni_cell_free out\n") ;
	*/
}

/*----------------------------------------------------------------------*/

/* Format a cell */
static int _uni_cell_format(
	int			vpi,	/* IN: VPI			*/
	int			vci,	/* IN: VCI			*/
	bool			islast,	/* IN: Flag for last cell	*/
	size_t			size,	/* IN: Payload size		*/
	byte *	 		pdata,	/* IN: Payload data		*/
	uni_cell_t *		pcell	/* OUT: Formated UNI cell	*/
) {
	int		lv_padd	= 0 ;
	byte *		lp_cell	= NULL ;

	/*
	DBG_OUT("_uni_cell_format in\n") ;
	*/

	/* Check Interface */
	if (!size || !pdata || !pcell || (size > ATM_CELL_PAYLOAD)) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	/* Normalize VPI & VCI */
	vpi &= 0x00ff ;
	vci &= 0xffff ;

	lp_cell = &(pcell->raw[0]) ;

	/* 
	 * Set the cell header
	 * 
	 * |--------|--------|
	 * |  GFC   |  hVPI  |
	 * |--------|--------|
	 * |  lVPI  | hhVCI  |
	 * |--------|--------|
	 * | hlVCI  | lhVCI  |
	 * |--------|--------|
	 * | llVCI  | PTI+CLP|
	 * |--------|--------|
	 * |       HEC       |
	 * |-----------------|
	 *
	 * GFC : Generic Flow Control (0x00 = uncontrolled)
	 * PTI : Payload Type Id (3 bits)
	 * 	= 0XY : User Data
	 * 		X = Congestion Bit (1 = congested)
	 * 		Y = User Signalling Bit (1 = last packet)
	 * 	= 100 : OAM Data / OAM Link Associated Cell
	 * 	= 101 : OAM Data / OAM End-to-End Associated Cell
	 * 	= 110 : OAM Data / Resource Management Cell
	 * 	= 111 : RESERVED FOR FUTURE USE
	 * CLP : Cell Loss Priority (1 bit)
	 * 	= 0 : high priority cell
	 * 	= 1 : low priority cell
	 * HEC : Header Error Control 
	 * 	= header 8 bit CRC polynomial x^8+x^2+x+1
	 *
	 * N.B : It seems that 0 for HEC works
	 */
	lp_cell[0] = (byte) ((0x0000)			| (vpi >> 4)) ;
	lp_cell[1] = (byte) ((vpi << 4)			| (vci >> 12)) ;
	lp_cell[2] = (byte) (vci >> 4) ;
	lp_cell[3] = (byte) (((vci & 0x0f) << 4)	| (islast ? 0x02 : 0x00)) ; 
	lp_cell[4] = (byte) (0x0000) ;

	/* Fill the Payload */
	lp_cell += ATM_CELL_HDR ;
	memcpy(lp_cell, pdata, size) ;

	/* Fill the padding if needed */
	if ((lv_padd = ATM_CELL_PAYLOAD - size) > 0)
		memset(lp_cell + size, 0, lv_padd) ;

	/*
	DBG_OUT("_uni_cell_format out\n") ;
	*/
	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Get VPI from cell */
static int _uni_cell_getVpi(
		uni_cell_t *		pcell	/* IN: the cell		*/
) {
	byte *	lp_raw = NULL ;
	int	lv_vpi ;

	/*
	DBG_OUT("_uni_cell_getVpi in\n") ;
	*/

	/* Check Interface */
	if (!pcell) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	lp_raw = pcell->raw ;

	lv_vpi = ((0x0f & lp_raw[0]) << 4) | (lp_raw[1] >> 4) ;

	/*
	DBG_OUT("_uni_cell_getVpi out\n") ;
	*/
	return lv_vpi ;
}

/*----------------------------------------------------------------------*/

/* Get VCI from cell */
static int _uni_cell_getVci(
		uni_cell_t *		pcell	/* IN: the cell		*/
) {
	byte *	lp_raw = NULL ;
	int	lv_vci ;
	
	/*
	DBG_OUT("_uni_cell_getVci in\n") ;
	*/

	/* Check Interface */
	if (!pcell) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	lp_raw = pcell->raw ;
	
	lv_vci = 
		((((0x0f & lp_raw[1]) << 4) | (lp_raw[2] >> 4)) << 8) |
	       	( ((0x0f & lp_raw[2]) << 4) | (lp_raw[3] >> 4)) ;

	/*
	DBG_OUT("_uni_cell_getVci out\n") ;
	*/
	return lv_vci ;
}

/*----------------------------------------------------------------------*/

/* check if this is the last cell */
static bool _uni_cell_islast(
		uni_cell_t *		pcell	/* IN: the cell		*/
) {
	bool lv_res ;
	/*
	DBG_OUT("_uni_cell_islast in\n") ;
	*/

	/* check interface */
	if (!pcell) {
		ERR_OUT("interface error\n") ;
		return true ;
	}

	lv_res = ((pcell->raw[3] & 0x02) == 0x02) ;

	/*
	DBG_OUT("_uni_cell_islast out\n") ;
	*/
	return lv_res ;
}

/*----------------------------------------------------------------------*/

/* Get The Payload */
static byte * _uni_cell_getPayload(
		uni_cell_t *		pcell	/* in: the cell		*/
) {
	/*
	DBG_OUT("_uni_cell_getPayload in\n") ;
	*/

	/* check interface */
	if (!pcell) {
		ERR_OUT("Interface Error\n") ;
		return NULL ;
	}

	/*
	DBG_OUT("_uni_cell_getPayload out\n") ;
	*/
	return &(pcell->raw[0]) ;
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 			UNI CELL QUEUE ABSTRACTION
  ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/* Init a queue */
static int _uni_cell_qinit(uni_cell_q_t *pcellq) {
	/*
	DBG_OUT("_uni_cell_qinit in\n") ;
	*/

	/* Check Interface */
	if (!pcellq) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}
	
	pcellq->nbcells		= 0 ;
	pcellq->first		= NULL ;
	pcellq->last		= NULL ;

	/*
	DBG_OUT("_uni_cell_qinit out\n") ;
	*/
	return 0 ;
}
/*----------------------------------------------------------------------*/

/* Alloc a new queue */
static uni_cell_q_t * _uni_cell_qalloc(void) {
	
	uni_cell_q_t * lp_cellq = NULL ;

	/*
	DBG_OUT("_uni_cell_qalloc in\n") ;
	*/
	lp_cellq = (uni_cell_q_t *) kmalloc(
			sizeof(uni_cell_q_t),
			GFP_KERNEL) ;
	if (!lp_cellq) {
		ERR_OUT("Not enought memory for queue\n") ;
		return NULL ;
	}

	/* Init fields */
	if (_uni_cell_qinit(lp_cellq)) {
		ERR_OUT("Queue init failed\n") ;
		kfree(lp_cellq) ;
		return NULL ;
	}
	
	/*
	DBG_OUT("_uni_cell_qalloc out\n") ;
	*/
	return lp_cellq ;
}

/*----------------------------------------------------------------------*/

/* 
 * Free a queue :
 * 	free also each remaining cells
 */
static void _uni_cell_qfree(uni_cell_q_t *pcellq) {

	uni_cell_t *	lp_cell 	= NULL ;
	uni_cell_t *	lp_cellnxt 	= NULL ;
	
	/*
	DBG_OUT("_uni_cell_qfree in\n") ;
	*/

	/* Check Interface */
	if (!pcellq) return ;

	/*
	 * Free each cells of the queue
	 */
	lp_cell = pcellq->first ;
	while (lp_cell) {
		lp_cellnxt	= lp_cell->next ;
		_uni_cell_free(lp_cell) ;
		lp_cell		= lp_cellnxt ;
	}

	/* Free Queue */
	kfree(pcellq) ;

	/*
	DBG_OUT("_uni_cell_qfree out\n") ;
	*/
}

/*----------------------------------------------------------------------*/

/*
 * Check if the queue is empty
 */
static bool _uni_cell_qisempty(
		uni_cell_q_t * pcellq
) {
	return (pcellq->nbcells == 0) ;
}

/*----------------------------------------------------------------------*/

/*
 * Push new UNI Cell in Queue
 */
static int _uni_cell_qpush(
		uni_cell_t *	pcell,	/* IN: Cell to insert	*/
		uni_cell_q_t *	pcellq	/* IN: Target queue	*/
) {
	/*
	DBG_OUT("_uni_cell_qpush in\n") ;
	*/

	/* Check Interface */
	if (!pcell || !pcellq) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	/* Add the new cell at the end of the list */
	pcell->next = NULL ;

	if (pcellq->last == NULL) {
		/* The queue is empty */
		pcellq->first = pcellq->last = pcell ;
	} else {
		pcellq->last->next 	= pcell ;
		pcellq->last		= pcell ;
	}
	pcellq->nbcells++ ;

	/*
	DBG_OUT("_uni_cell_qpush out\n") ;
	*/
	return 0 ;
}

/*----------------------------------------------------------------------*/

/*
 * Pop an UNI Cell from Queue :
 * 	return NULL when empty or invalid queue
 */
static uni_cell_t * _uni_cell_qpop(
		uni_cell_q_t *	pcellq	/* IN: Target queue	*/
) {
	uni_cell_t * lp_cell = NULL ;
	
	/*
	DBG_OUT("_uni_cell_qpop in\n") ;
	*/

	/* Check Interface */
	if (!pcellq) {
		ERR_OUT("Interface Error\n") ;
		return NULL ;
	}

	/* Remove first cell in list */
	if (pcellq->first) {
		lp_cell 	= pcellq->first ;
		pcellq->first 	= lp_cell->next ;
		lp_cell->next 	= NULL ;
		pcellq->nbcells-- ;
	}
	
	/*
	DBG_OUT("_uni_cell_qpop out\n") ;
	*/
	return lp_cell ;
}

/*----------------------------------------------------------------------*/

/*
 * Get the UNI Cell on head Queue :
 * 	return NULL when empty or invalid queue
 */
static uni_cell_t * _uni_cell_qhead(
		uni_cell_q_t *	pcellq	/* IN: Target queue	*/
) {
	uni_cell_t * lp_cell = NULL ;
	
	/*
	DBG_OUT("_uni_cell_qhead in\n") ;
	*/

	/* Check Interface */
	if (!pcellq) {
		ERR_OUT("Interface Error\n") ;
		return NULL ;
	}

	lp_cell = pcellq->first ;

	/*
	DBG_OUT("_uni_cell_qhead out\n") ;
	*/
	return lp_cell ;
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 			AAL5 FRAME ABSTRACTION
  ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/* Init a new AAL5 Frame */
static int _aal5_init(aal5_t * paal5) {
	
	if (!paal5) return -EINVAL ;

	paal5->is_complete	= false ;
	paal5->is_valid		= false ;
	paal5->vci		= -1 ;
	paal5->vpi		= -1 ;
	paal5->nbcells		= 0 ;
	paal5->pccell		= NULL ;
	paal5->plastcell	= NULL ;
	paal5->pthecells	= NULL ;

	paal5->_crc		= -1 ;

	return 0 ;
}
	
	
/*----------------------------------------------------------------------*/

/* Alloc a new AAL5 frame */
static aal5_t * _aal5_alloc(void) {
	aal5_t * lp_aal5 = NULL ;

	lp_aal5 = kmalloc(sizeof(aal5_t), GFP_KERNEL) ;
	if (lp_aal5) return NULL ;
	
	if (_aal5_init(lp_aal5)) {
	       kfree(lp_aal5) ;
	       return NULL ;
	}

	return lp_aal5 ;
}

/*----------------------------------------------------------------------*/

/* Free an AAL5 frame */
static void _aal5_free(aal5_t *paal5) {
	uni_cell_t *	lp_cell = NULL ;
	
	if (!paal5) return ;

	/*
	 * Free all Cells
	 */
	while ((lp_cell = paal5->pthecells)) {
		paal5->pthecells = lp_cell->next ;
		_uni_cell_free(lp_cell) ;
	}

	kfree(paal5) ;

}	

/*----------------------------------------------------------------------*/

/* Reset a AA5 frame */
static void _aal5_clean(aal5_t *paal5) {
	uni_cell_t *	lp_cell = NULL ;

	if (!paal5) return ;

	/*
	 * Free all Cells
	 */
	while ((lp_cell = paal5->pthecells)) {
		paal5->pthecells = lp_cell->next ;
		_uni_cell_free(lp_cell) ;
	}

	_aal5_init(paal5) ;
}

/*----------------------------------------------------------------------*/

/* Get VPI from AAL5 */
static int _aal5_getVpi(
	aal5_t * paal5			/* IN: the frame		*/
) {
	if (!paal5 || !paal5->is_valid) return -1 ;
	return paal5->vpi ;
}

/*----------------------------------------------------------------------*/

/* Get VCI from AAL5 */
static int _aal5_getVci(
	aal5_t * paal5			/* IN: the frame		*/
) {
	if (!paal5 || !paal5->is_valid) return -1 ;
	return paal5->vci ;
}

/*----------------------------------------------------------------------*/

/* check if this is the frame is completed */
static bool _aal5_iscomplete(
	aal5_t * paal5			/* IN: the frame		*/
) {
	return (paal5 ? paal5->is_complete : false) ;
}	

/*----------------------------------------------------------------------*/

/* check if this is the frame is valid */
static bool _aal5_isvalid(
	aal5_t * paal5			/* IN: the frame		*/
) {
	return (paal5 && paal5->is_complete ? paal5->is_valid : false) ;
}

/*----------------------------------------------------------------------*/

/* Append a new cell */
static int _aal5_add_cell(
	aal5_t *	paal5,		/* IN: the frame		*/
	uni_cell_t *	pcell		/* IN: the new cell		*/
) {
	size_t		lv_length ;
	unsigned int	lv_crc ;
	if (!paal5 || !pcell || paal5->is_complete)
		return -EINVAL ;

	pcell->next = NULL ;

	if (!_uni_cell_islast(pcell)) {
		/* compute crc */
		paal5->_crc 	= ~ _calc_crc(
					pcell->raw + ATM_CELL_HDR,
					ATM_CELL_PAYLOAD,
					paal5->_crc) ;
	} else {
		paal5->is_complete = true ;
		
		/* check given length */
		lv_length = (pcell->raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD - 6] << 8)
			+ (pcell->raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD - 5]) ;
		lv_length = (lv_length + 8 + 
				ATM_CELL_PAYLOAD - 1) / ATM_CELL_PAYLOAD ;
		if (lv_length != (paal5->nbcells + 1)) {
			ERR_OUT("Invalid length field in trailed\n") ;
			return -EINVAL ;
		}

		/* compute final crc */
		paal5->_crc 	= ~ _calc_crc(
					pcell->raw + ATM_CELL_HDR,
					ATM_CELL_PAYLOAD - 4,
					paal5->_crc) ;

		/* compare to given crc */
		lv_crc = (pcell->raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD - 4] << 24)
			+ (pcell->raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD - 3] << 12)
			+ (pcell->raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD - 2] << 8)
			+ (pcell->raw[ATM_CELL_HDR + ATM_CELL_PAYLOAD - 1]) ;

		if (lv_crc != paal5->_crc) {
			ERR_OUT("Invalid crc field in trailer\n") ;
			return -EINVAL ;
		}
	}

	if (!paal5->nbcells) {
		paal5->pthecells 	= pcell ; 
		paal5->vpi		= _uni_cell_getVpi(pcell) ;
		paal5->vci		= _uni_cell_getVci(pcell) ;
	} else {
		/* check vci & vpi */
		if ((_uni_cell_getVpi(pcell) != paal5->vpi) ||
				(_uni_cell_getVci(pcell) != paal5->vci)) {
			ERR_OUT("Incoherent cell for this AAL5 frame\n") ;
			return -EINVAL ;
		}
		paal5->plastcell->next	= pcell ;
	}

	paal5->plastcell = pcell ;
	
	paal5->nbcells++ ;

	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Get Next Cell of the AAL5 frame */
static uni_cell_t * _aal5_get_next(aal5_t *paal5) {
	
	uni_cell_t * lp_cell ;
	
	if (!paal5 || !paal5->is_complete)
		return NULL ;

	if (!paal5->pccell) {
		/* First time */
		paal5->pccell = paal5->pthecells ;
		lp_cell = paal5->pccell ;
	} else if ((lp_cell = paal5->pccell->next)) {
		/* Not the last yet */
		paal5->pccell = lp_cell ;
	}

	return lp_cell ;
}

/*----------------------------------------------------------------------*/
