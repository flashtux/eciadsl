/*****************************************************************************
*                                                                            *
*     Driver pour le modem ADSL ECI HiFocus utilise par France Telecom       *
*                                                                            *
*     Author : Valette Jean-Sebastien <jean-sebastien.valette@free.fr        *
*              Eric Bardes  <email@fournisseur>                              *
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

#include <linux/modversions.h>
#include <linux/module.h>

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
#include <asm/uaccess.h>

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
		KERN_DEBUG __FILE__ " " fmt, \
		##argz \
	)

#define DUMP_LINES 	64

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
	unsigned char	lv_format[( (3+3+3*8+1+8+1)  * DUMP_LINES) + 1] ; 
/*
	line = <7>XX:dd dd dd dd dd dd dd dd:aaaaaaaa\n 
	size=  3 | 3| 3 * 8                |1| 8     |1
*/
	unsigned char *	lp_cur = lv_format ;
	int 		lc_1 ;
	int		lc_2 ;
	int		lv_max = (size > DUMP_LINES * 8 ? 
					DUMP_LINES * 8 : 
					size) ;
  
	lv_format[0] = '\0' ;

	/* Format the buffer */
	for (lc_1=0 ; lc_1 < lv_max ; lc_1 += 8) {
		lp_cur += sprintf(lp_cur, "%s:", KERN_DEBUG);
		
		/* Print hexa */
		for (
			lc_2 = lc_1 ; 
			((lc_2 < lv_max) && (lc_2 < lc_1 + 8)) ;
			lc_2++
		) {
			sprintf(lp_cur, "%02x ", buffer[lc_2]) ;
			lp_cur 	+= 3 ; /* wrote 3 chars */
		}
		/* Print Padding */
		for (;lc_2 < lc_1 + 8 ; lc_2++) {
			sprintf(lp_cur, "   ");
			lp_cur 	+= 3 ; /* wrote 3 chars */
		}

		/* Print separator */
		sprintf(lp_cur, "- ") ;
		lp_cur += 2 ;

		/* Print ASCII */
		for (
				lc_2 = lc_1;
				(lc_2 < lv_max) && (lc_2 < lc_1 + 8) ;
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
		KERN_DEBUG "ECI_USB [%s:%d] : %s [%d] %s\n%s\n",
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
#define ATM_CELL_SZ \
	(size_t)ATM_CELL_HDR + ATM_CELL_PAYLOAD	/* Full Cell Length	*/

/*-- T Y P E S ---------------------------------------------------------*/

struct uni_cell ;				/* forward declaration	*/
typedef struct uni_cell uni_cell_t ;

struct uni_cell {
	/*
	 * Do not insert any thing before
	 * To keep &uni_cell = &raw
	 */
	byte 		raw[ATM_CELL_SZ] ;

	/* -- for queue managements -- */
	uni_cell_t * 	next ;
} ;



/*-- C O N S T A N T S -------------------------------------------------*/
/*-- V A R I A B L E S -------------------------------------------------*/
/*-- F U N C T I O N S -------------------------------------------------*/

/* Alloc a new UNI Cell */
static uni_cell_t * _uni_cell_alloc(void) ;

/* Create New Cell from raw */
static uni_cell_t * _uni_cell_fromRaw(size_t, byte*) ;

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
static inline int _uni_cell_getVpi(
	uni_cell_t *		/* IN: the cell			*/
) ;

/* Get VCI from cell (-1 if error) */
static inline int _uni_cell_getVci(
	uni_cell_t *		/* IN: the cell			*/
) ;

/* check if this is the last cell (true on error) */
static inline bool _uni_cell_islast(
	uni_cell_t *		/* in: the cell			*/
) ;

/* Get The Payload */
static inline byte * _uni_cell_getPayload(
	uni_cell_t *		/* in: the cell			*/
) ;

/* Set HEC of header */
static void _uni_cell_setHEC(
	uni_cell_t *		/* in: the cell			*/
) ;

/* Check HEC of header */
static bool _uni_cell_chkHEC(
	byte *			/* in: raw cell data		*/
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

/*--------------------- U N I  C E L L  L I S T ------------------------*/

/*-- M A C R O S -------------------------------------------------------*/
/*-- T Y P E S ---------------------------------------------------------*/

struct uni_cell_list {
	unsigned int		nbcells ;	/* Number of cells	*/
	uni_cell_t *		first ;		/* First list item	*/
	uni_cell_t *		last ;		/* Last list item	*/
} ;

typedef struct uni_cell_list uni_cell_list_t ;

typedef const uni_cell_t * uni_cell_list_crs_t ;

/*-- C O N S T A N T S -------------------------------------------------*/
/*-- V A R I A B L E S -------------------------------------------------*/
/*-- F U N C T I O N S -------------------------------------------------*/

/* Init a list */
static int _uni_cell_list_init(uni_cell_list_t *) ;

/* Reset a list */
static void _uni_cell_list_reset(uni_cell_list_t *) ;

/* Alloc a new list */
static uni_cell_list_t * _uni_cell_list_alloc(void) ;

/* Free a list (free each cells) */
static void _uni_cell_list_free(uni_cell_list_t *) ;

/* Get number of Cells in list (<0 : error)*/
static inline int _uni_cell_list_nbcells(uni_cell_list_t *) ;

/* Get First cell of list */
static inline const uni_cell_t * _uni_cell_list_first(uni_cell_list_t *) ;

/* Get Last cell of list */
static inline const uni_cell_t * _uni_cell_list_last(uni_cell_list_t *) ;

/* Extract First cell of list */
static uni_cell_t * _uni_cell_list_extract(uni_cell_list_t *) ;

/* Insert a new cell at the beginning of the list */
static int _uni_cell_list_insert(uni_cell_list_t *, uni_cell_t *) ;

/* Add a new cell at the end of the list */
static int _uni_cell_list_append(uni_cell_list_t *, uni_cell_t *) ;

/* Concatenate 2 lists (reset second list) */
static int _uni_cell_list_cat(uni_cell_list_t*, uni_cell_list_t*) ;

/* Move cursor forward */
static inline int _uni_cell_list_crs_next(uni_cell_list_crs_t *) ;

/*
 *
 * Split raw AAL5 frame into UNI cells
 * We need to format the AAL5 trailer
 *
 */
static int _aal5_to_cells(
		int,				/* IN: VPI		*/
		int,				/* IN: VCI		*/
		size_t,				/* IN: AAL5 size	*/
		byte*,				/* IN: AAL5 raw data	*/
		uni_cell_list_t*			/* IN/OUT: filled cells */
) ;

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
static inline int _aal5_getVpi(
	aal5_t *		/* IN: the frame		*/
) ;

/* Get VCI from AAL5 */
static inline int _aal5_getVci(
	aal5_t *		/* IN: the frame		*/
) ;

/* Get AAL5 size */
static inline size_t _aal5_getSize(
	aal5_t *		/* IN: the frame		*/
) ;
	
/* check if this is the frame is completed */
static inline bool _aal5_iscomplete(
	aal5_t *		/* IN: the frame		*/
) ;

/* check if this is the frame is valid */
static inline bool _aal5_isvalid(
	aal5_t *		/* IN: the frame		*/
) ;

/* Get the number of cells in the AAL5 frame */
static inline int _aal5_getNbCell(aal5_t*) ;

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
static int eci_atm_ioctl(struct atm_dev *dev,unsigned int cmd, void *arg);
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb);

/* -- Private Functions -- */
static u32 _calc_crc(unsigned char *, int, unsigned int) ;
static int _eci_tx_aal5( 
		int, int, 
		struct eci_instance *, struct sk_buff *
) ;
static int _eci_rx_aal5(
		struct eci_instance*,
		aal5_t*
) ;

/*
 * Send data from USB (single uni cell) to ATM
 */
static int _eci_usb_rx(
	uni_cell_t *		/* IN: UNI Cell		*/
) ;
static int eci_atm_receive_cell(
		struct eci_instance *,
		uni_cell_list_t *
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
static int eci_usb_send_urb(struct eci_instance *instance, 
			struct uni_cell_list *cells);


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
	struct urb		*bulk_urb;	/*	INT pooling	*/
	unsigned char 		*interrupt_buffer;
	unsigned char 		*pooling_buffer;
	char			iso_celbuf[ATM_CELL_SZ]; /* incomplete cell */
	int			iso_celbuf_pos;/*	Pos in cell buf	*/

	/* -- test -- */
	struct atm_vcc		*pcurvcc ;
 	struct aal5		*pbklogaal5;	/*	AAL5 to complete */
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
			out_instance->iso_celbuf_pos = 0 ;
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
			Allocate bulk urb	
		*/
		if(!(out_instance->bulk_urb = usb_alloc_urb(0)))
		{
			ERR_OUT("Can't alloacate bulk URB\n");
			return(0);
		}
		DBG_OUT("Vendor Stuff\n");
		/*
			Just Reset EP 0x02 (no reset)
			if some one day, all endpoint before sending any urb
			Send vendors URB 
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

	out_instance->pcurvcc 		= NULL ;
	out_instance->pbklogaal5	= NULL ;

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
		if (eci_instances->pbklogaal5)
			_aal5_free(eci_instances->pbklogaal5) ;
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
	DBG_OUT("code : %d\n", code) ;
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

	/* test */
	((struct eci_instance*)vcc->dev->dev_data)->pcurvcc = vcc ;
	
	DBG_OUT("eci_atm_open out\n");

	return 0;
};

/*----------------------------------------------------------------------*/
static void eci_atm_close(struct atm_vcc *vcc)
{
	struct eci_instance * lp_instance = 
		(struct eci_instance*)vcc->dev->dev_data ;
	DBG_OUT("eci_atm_close in\n");
	lp_instance->pcurvcc = NULL ;
	if (lp_instance->pbklogaal5) {
		_aal5_free(lp_instance->pbklogaal5) ;
		lp_instance->pbklogaal5 = NULL ;
	}
	DBG_OUT("eci_atm_close out\n");
};

/*----------------------------------------------------------------------*/
static int eci_atm_ioctl(struct atm_dev *dev,unsigned int cmd, void *arg)
{
	int lv_res = -EINVAL ;

	DBG_OUT("eci_atm_ioctl in\n");
	switch (cmd) {
		case ATM_QUERYLOOP:
			lv_res = put_user(ATM_LM_NONE, (int*)arg) ? -EFAULT : 0;
			break ;
		default:
			DBG_OUT("not supported IOCTL[%u]\n", cmd) ;
			lv_res =  -ENOIOCTLCMD;
			break ;
	}
	DBG_OUT("eci_atm_ioctl out\n");
	return lv_res ;
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
			/*DBG_OUT("EP INT received 64 bytes\n"); 
			DBG_RAW_OUT("EP INT datas :",in_buf, 64); */
		}
	}
	/*	DBG_OUT("Int callBack out\n");	*/
}

static void eci_iso_callback(struct urb *urb)
{
	struct eci_instance 	*instance;	/* Driver private structre */
	int 			i;		/* Frame Counter	*/
 	int 			pos;		/* Buffer curent pos counter */
 	struct uni_cell_list	*cells;		/* New computed queue of cell */
	struct uni_cell 	*cell;		/* Working cell		*/
	unsigned char 		*buf;		/* Working buffer pointer */

	instance = (struct eci_instance *)urb->context;
	if ((!urb->status || urb->status == EREMOTEIO)  && urb->actual_length)
	{
 		if(!(cells = _uni_cell_list_alloc())) {
			ERR_OUT("Failed to allocate cell list\n");
			goto end;
		}
		for (i=0;i<ECI_NB_ISO_PACKET;i++)
		{
			if (!urb->iso_frame_desc[i].status &&
				 urb->iso_frame_desc[i].actual_length)
			{

				DBG_OUT("Data available in frame [%d]\n", i) ;
				DBG_RAW_OUT(
					"received data", 
					urb->transfer_buffer +
						urb->iso_frame_desc[i].offset,
					urb->iso_frame_desc[i].actual_length);
				if(instance->iso_celbuf_pos)
				{
					pos = ATM_CELL_SZ - instance->iso_celbuf_pos;
					memcpy(instance->iso_celbuf + 
					       instance->iso_celbuf_pos,
						urb->transfer_buffer +
						urb->iso_frame_desc[i].offset, 
						pos) ;
					if(!(cell = _uni_cell_fromRaw(
						 ATM_CELL_SZ,
						 instance->iso_celbuf)))
					{
				 	  ERR_OUT(
					  "not enough memory for new cell\n");
					}
					else 
					{
						if(_uni_cell_list_append(
							cells, cell))
						{
						    ERR_OUT(
				 		"Couldn't queue One cell\n");
						    _uni_cell_free(cell);
						}
					}
				}
				else
				{
					pos = 0 ;
				}
				for(buf=urb->transfer_buffer +
					urb->iso_frame_desc[i].offset;
					pos + ATM_CELL_SZ <=
					urb->iso_frame_desc[i].actual_length;
					pos+=ATM_CELL_SZ)
				{
					cell = _uni_cell_fromRaw(ATM_CELL_SZ,
						buf + pos);
					if (!cell)
					{
						ERR_OUT(
					   "not enough mem for new cell\n") ;
					}
					else 
					 if(_uni_cell_list_append(cells,cell))
					 {
					   ERR_OUT("Couldn't queue One cell\n");
					   _uni_cell_free(cell);
					 }
				}
				if((instance->iso_celbuf_pos = 
					urb->iso_frame_desc[i].actual_length - 
					pos))
				{
					DBG_OUT("Saving data for next frame\n");
					memcpy(instance->iso_celbuf, buf +pos,
						instance->iso_celbuf_pos);
				}
			}
		}
		DBG_OUT("Received cell, send to atm\n");
 		if(eci_atm_receive_cell(instance, cells)) {
			_uni_cell_list_free(cells);
		}
	}
					
	end:
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

static int eci_usb_send_urb(struct eci_instance *instance, 
			struct uni_cell_list *cells)
{
	int 		ret;		/* counter			*/
	/*not used:int 		nbcell;*/	/* cellcount expected		*/
	unsigned	char *buf;	/* urb buffer			*/
	int		buflen;		/* urb buffer len		*/
	int		bufpos;		/* position in urb buffer	*/
	int		nbcells;	/* number of cell to be sent 	*/
	uni_cell_list_crs_t	cell;	/* current computed cell	*/
	struct urb	*urb;		/* urb pointer to sent urb	*/

	nbcells = _uni_cell_list_nbcells(cells);
	buflen = ATM_CELL_SZ * nbcells;
	/*	urb = instance->bulk_urb;	*/
	if(!(urb = usb_alloc_urb(0)))
	{
		ERR_OUT("Can't alloc urb !!\n");
		return(0);
	}
	if(!(buf=(unsigned char *)kmalloc(buflen, GFP_KERNEL)))
	{
		ERR_OUT("Can't allocate bulk urb Buffer\n");
		return(0);
	}
	bufpos = ret = 0;
 	cell = _uni_cell_list_first(cells);
 	while(ret < nbcells)
	{
 		if(cell)
		{
			memcpy(buf + bufpos, cell->raw, ATM_CELL_SZ);
			bufpos += ATM_CELL_SZ;
			ret++;
		}
		else
			break;
		if(_uni_cell_list_crs_next(&cell)) break;
	}
	if( ret != nbcells)
	{
		ERR_OUT("Error computing bulk buffer\n");
		ERR_OUT("Expecting %d cell got %d\n", nbcells, ret);
	}
	DBG_RAW_OUT("new bulk",buf,bufpos);
	FILL_BULK_URB(urb, instance->usb_wan_dev, 
		usb_sndbulkpipe(instance->usb_wan_dev, ECI_BULK_PIPE),
		buf, bufpos, eci_bulk_callback, instance);
	urb->transfer_flags = USB_QUEUE_BULK;
	if(usb_submit_urb(urb))
	{
		ERR_OUT("Can't submit bulk urb\n");
		kfree(buf);
		return(0);
	}
	return(ret);	
}

static void eci_bulk_callback(struct urb *urb)
{
	if (!urb) return ;/*	????	*/
	if(urb->status)
	{
		ERR_OUT("Error on Bulk URB, status %d\n", urb->status);
	}
	kfree(urb->transfer_buffer);
	usb_free_urb(urb);
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
	int		lv_rc 		= 0 ;
	int		lv_nbsent ;
	uni_cell_list_t 	lv_list ;
	
	/* Check Interface */
	if (!pinstance || !pskb) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	lv_rc = _uni_cell_list_init(&lv_list) ;
	if (lv_rc) return lv_rc ;

	/* Split AAL5 frame */
	lv_rc = _aal5_to_cells(
			vpi,
			vci,
			pskb->len,
			pskb->data,
			&lv_list) ;
	if (lv_rc) {
		ERR_OUT("Failed to split aal5 frame\n") ;
		_uni_cell_list_reset(&lv_list) ;
		return lv_rc ;
	}

	/* Send the Cells to USB layer */
	lv_nbsent = eci_usb_send_urb(pinstance, &lv_list) ;

	if (!lv_nbsent || lv_nbsent != _uni_cell_list_nbcells(&lv_list)) {
	       ERR_OUT(
			       "Not all the cells where sent (%d/%d)\n",
			       lv_nbsent,
			       _uni_cell_list_nbcells(&lv_list)) ;
	       lv_rc = -1 ;
	}

	_uni_cell_list_reset(&lv_list) ;

	return lv_rc ;
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
 * Send Cell to ATM
 *
 */
static int eci_atm_receive_cell(
		struct eci_instance * pinstance,
		uni_cell_list_t *		plist
) {
	uni_cell_t *		lp_cell		= NULL ;
 	aal5_t *		lp_aal5 	= NULL ;
	int			lv_rc ;
	
	/* Check Interface */
	if (!pinstance || !plist)
		return -EINVAL ;

	/* Check if VCC available */
	if (!pinstance->pcurvcc){
		ERR_OUT("No opened VC\n") ;
		return -ENXIO ;
	}

 	/* Manage backlog AAL5 */
 	if (pinstance->pbklogaal5) {
 		lp_aal5 = pinstance->pbklogaal5 ;
 		pinstance->pbklogaal5 = NULL ;
 	} else {
 		lp_aal5 = _aal5_alloc() ;
 		if (!lp_aal5) {
 			ERR_OUT("AAL5 init failed\n") ;
 			return -ENOMEM ;
 		}
	}

	/* Parse The complete list */
	do {

		/* Manage a complete AAL5 */
		do {
			lp_cell = _uni_cell_list_extract(plist) ;
			if (!lp_cell) {
				DBG_OUT("No more cell\n") ;
				goto end;
			}

			/* Check that the Cell is PDU */
			if ((lp_cell->raw[3] >> 3) & 0x1) {
				WRN_OUT("Not supported cell type\n") ;
				DBG_RAW_OUT("the unsupported cell",
						lp_cell->raw,
						ATM_CELL_SZ) ;
				_uni_cell_free(lp_cell) ;
				continue ;
			}
			
			lv_rc = _aal5_add_cell(lp_aal5, lp_cell) ;
			if (lv_rc) {
				ERR_OUT("Failed to add new cell drop frame\n") ;
				_uni_cell_free(lp_cell) ;
				_aal5_clean(lp_aal5) ;
				goto end;
			}
		} while (!_aal5_iscomplete(lp_aal5)) ;

		/* Now send it to ATM layer */
		_eci_rx_aal5(pinstance, lp_aal5) ;

		/* Reset AAL5 */
		_aal5_clean(lp_aal5) ;
	} while (true) ;
		
	end:

 	/* Manage AAL5 backlog */
 	if (_aal5_getNbCell(lp_aal5) && !_aal5_iscomplete(lp_aal5)) {
 		pinstance->pbklogaal5 = lp_aal5 ;
 		DBG_OUT(
 			"store bklog AAL5 with [%d] cells\n",
 			_aal5_getNbCell(lp_aal5)) ;
 	} else {
 		_aal5_free(lp_aal5) ;
 	}
	_uni_cell_list_free(plist) ;
	return 0 ;
}

/*----------------------------------------------------------------------
 * 
 * Send data to ATM
 *
 */
static int _eci_rx_aal5(
		struct eci_instance *	pinstance,
		aal5_t *		paal5
) {
	uni_cell_t *		lp_cell		= NULL ;
	int			lv_vpi ;
	int			lv_vci ;
	size_t			lv_size ;
	struct sk_buff *	lp_skb		= NULL ;
	byte *			lp_data		= NULL ;

	/* Check Interface */
	if (!pinstance || !paal5) return -EINVAL ;

	lv_vpi = _aal5_getVpi(paal5) ;
	lv_vci = _aal5_getVci(paal5) ;

	if (lv_vpi == -1 || lv_vci == -1) {
		ERR_OUT("VC info not properly set\n") ;
		return -EINVAL ;
	}

	lv_size = _aal5_getSize(paal5) ;
	
	/* Alloc SKB */
	lp_skb = atm_alloc_charge(pinstance->pcurvcc, lv_size, GFP_ATOMIC);
	if (!lp_skb) {
		ERR_OUT("not enought mem for new skb\n") ;
		return -ENOMEM ;
	}

	/* Init SKB */
	skb_put(lp_skb, lv_size) ;
	ATM_SKB(lp_skb)->vcc = pinstance->pcurvcc ;
	lp_skb->stamp = xtime ;

	/* Copy data */
	lp_data = lp_skb->data ;
	while (lv_size > ATM_CELL_PAYLOAD) {
		lp_cell = _aal5_get_next(paal5) ;
		if (!lp_cell) {
			ERR_OUT("Next Cell not availlable\n") ;
			/* TODO : free SKB */
			return -1 ;
		}
		memcpy(
			lp_data, 
			_uni_cell_getPayload(lp_cell), 
			ATM_CELL_PAYLOAD) ;
		lp_data += ATM_CELL_PAYLOAD ;
		lv_size -= ATM_CELL_PAYLOAD ;
	}

	/* Copy last bytes if any */
	lp_cell = _aal5_get_next(paal5) ;
	if (!lp_cell) {
		ERR_OUT("Next Cell not availlable\n") ;
		/* TODO : free SKB */
		return -1 ;
	}
	memcpy(
		lp_data, 
		_uni_cell_getPayload(lp_cell), 
		lv_size) ;

	DBG_RAW_OUT("aal5 to be sent", lp_skb->data, lp_skb->len) ;

	DBG_OUT("SEND AAL5 TO ATM\n") ;
	pinstance->pcurvcc->push(pinstance->pcurvcc, lp_skb) ;
	atomic_inc(&pinstance->pcurvcc->stats->rx) ;
	

	return 0 ;

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
				GFP_ATOMIC) ;
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
 * Built a new UNI cell from raw data
 * Warning : buffer size MUST be ATM_CELL_SZ bytes
 */
static uni_cell_t * _uni_cell_fromRaw(
		size_t	szbuffer,	/* IN: buffer length = 53	*/
		byte *	buffer		/* IN: Raw Cell data		*/
) {
	uni_cell_t * lp_cell = NULL ;
	
	/* Check interface */
	if (!buffer || (szbuffer != ATM_CELL_SZ)) {
		ERR_OUT("_uni_cell_fromRaw interface error\n") ;
		return NULL ;
	}

	/* Check raw cell header */
	if (!_uni_cell_chkHEC(buffer)) {
		ERR_OUT("Corrupted header\n") ;
		return NULL ;
	}
	
	/* Alloc new cell */
	lp_cell = _uni_cell_alloc() ;
	if (!lp_cell) {
		ERR_OUT("Not enought mem for new cell\n") ;
		return NULL ;
	}

	/* Copy data */
	memcpy(lp_cell->raw, buffer, szbuffer) ;

	lp_cell->next = NULL ;

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
	 */
	lp_cell[0] = (byte) ((0x0000)			| (vpi >> 4)) ;
	lp_cell[1] = (byte) ((vpi << 4)			| (vci >> 12)) ;
	lp_cell[2] = (byte) (vci >> 4) ;
	lp_cell[3] = (byte) (((vci & 0x0f) << 4)	| (islast ? 0x02 : 0x00)) ; 
	lp_cell[4] = (byte) (0x0000) ;

	/* Fill HEC */
	_uni_cell_setHEC(pcell) ;

	/* Fill the Payload */
	lp_cell += ATM_CELL_HDR ;
	memcpy(lp_cell, pdata, size) ;

	/* Fill the padding if needed */
	lv_padd = ATM_CELL_PAYLOAD - size ;
	if (lv_padd > 0)
		memset(lp_cell + size, 0, lv_padd) ;

	/*
	DBG_OUT("_uni_cell_format out\n") ;
	*/
	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Get VPI from cell */
static inline int _uni_cell_getVpi(
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
static inline int _uni_cell_getVci(
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
static inline bool _uni_cell_islast(
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
static inline byte * _uni_cell_getPayload(
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
	return &(pcell->raw[0]) + ATM_CELL_HDR ;
}

/*----------------------------------------------------------------------*/

#define COSET_LEADER (unsigned int) 0x055	/* x^6 + x^4 + x^2 + 1  */

/* Array of CRC codes for HEC */
static unsigned char _syndrome_table[] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
	0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
	0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
	0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
	0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
	0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
	0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
	0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
	0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
	0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
	0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
	0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
	0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
	0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
	0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
	0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
	0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
	0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
	0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
	0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
	0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
	0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
	0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
	0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
	0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
	0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
	0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
	0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
	0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
	0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
	0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
	0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
} ;

/* Error position in the header */
static int _err_pos[] = {
	-128,   39,   38,  128,   37,  128,  128,   31,
	  36,  128,  128,    8,  128,  128,   30,  128,
	  35,  128,  128,  128,  128,   23,    7,  128,
	 128,  128,  128,  128,   29,  128,  128,  128,
	  34,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,   22,  128,    6,  128,  128,  128,
	 128,    0,  128,  128,  128,  128,  128,  128,
	  28,  128,  128,  128,  128,  128,  128,  128,
	  33,  128,  128,   10,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	 128,   12,  128,  128,   21,  128,  128,   19,
	   5,  128,  128,   17,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,    3,
	 128,  128,  128,   15,  128,  128,  128,  128,
	  27,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	  32,  128,  128,  128,  128,  128,    9,  128,
	 128,   24,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,    1,  128,  128,  128,  128,
	 128,  128,   11,  128,  128,  128,  128,  128,
	  20,  128,  128,   13,  128,  128,   18,  128,
	   4,  128,  128,  128,  128,  128,   16,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,   25,
	 128,  128,  128,  128,  128,  128,    2,  128,
	 128,  128,  128,  128,  128,  128,   14,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	  26,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,  128,
	 128,  128,  128,  128,  128,  128,  128,  128 
} ;

/* Set HEC of header */
static void _uni_cell_setHEC(
	uni_cell_t *	pcell	/* in: the cell			*/
) {
	byte	lv_accum = 0 ;
	int	i ;

	/* check interface */
	if (!pcell) {
		ERR_OUT("Interface error\n") ;
		return ;
	}

	for (i = 0; i < 4; i++)
		lv_accum = _syndrome_table[ lv_accum ^ pcell->raw[i] ] ;
	pcell->raw[4] = lv_accum ^ COSET_LEADER ;
}

/*----------------------------------------------------------------------*/

/* Check HEC of header */
static bool _uni_cell_chkHEC(
	byte *	raw			/* in: raw cell data		*/
) {
	byte	lv_syndrome ;
	int	i ;
	int	lv_err_pos ;

	/* check interface */
	if (!raw) {
		ERR_OUT("Interface error\n") ;
		return false ;
	}

	/* Compute header HEC */
	lv_syndrome = 0;
	for (i = 0; i < 4; i++)
		lv_syndrome = _syndrome_table[ lv_syndrome ^ raw[i] ] ;
	lv_syndrome ^= raw[4] ^ COSET_LEADER ;

	lv_err_pos = _err_pos[lv_syndrome] ;

	if (lv_err_pos < 0)
	{
		return true ;
	}
	else if (lv_err_pos < 40)
	{
		raw[lv_err_pos / 8] ^= (0x80 >> (lv_err_pos % 8));
		DBG_OUT(
			"Error corrected in cell header for bit [%d]\n",
			lv_err_pos) ;
		return true ;
	}
	else
	{
		ERR_OUT("Uncorrectible error in cell header\n") ;
		return false ;
	}
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
 			UNI CELL LIST ABSTRACTION
  ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/* Init a list */
static int _uni_cell_list_init(uni_cell_list_t *plist) {
	if (!plist) return -EINVAL ;
	memset(plist, 0x00, sizeof(uni_cell_list_t)) ;
	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Reset a list (free each cells) */

static void _uni_cell_list_reset(uni_cell_list_t *plist) {
	uni_cell_t *	lp_cell = NULL ;

	if (!plist) return ;

	/* Free each cells of the list */
	while ((lp_cell = _uni_cell_list_extract(plist))) {
		_uni_cell_free(lp_cell) ;
	}
	
	plist->nbcells	= 0 ;
	plist->first	= NULL ;
	plist->last	= NULL ;

}


/*----------------------------------------------------------------------*/

/* Alloc a new list */

static uni_cell_list_t * _uni_cell_list_alloc(void) {
	uni_cell_list_t * lp_list = NULL ;

	lp_list = kmalloc(sizeof(uni_cell_list_t), GFP_ATOMIC) ;
	if (!lp_list) return NULL ;

	if (_uni_cell_list_init(lp_list)) {
		kfree(lp_list) ;
		return NULL ;
	}
	return lp_list ;
}

/*----------------------------------------------------------------------*/

/* Free a list (free each cells) */

static void _uni_cell_list_free(uni_cell_list_t *plist) {

	if (!plist) return ;

	/* Free each cells of the list */
	_uni_cell_list_reset(plist) ;

	kfree(plist) ;
}

/*----------------------------------------------------------------------*/

/* Get number of Cells in list (<0 : error)*/

static inline int _uni_cell_list_nbcells(uni_cell_list_t *plist) {
	return (plist 
			? plist->nbcells 
			: -EINVAL) ;
}

/*----------------------------------------------------------------------*/

/* Get First cell of list */
static inline const uni_cell_t * _uni_cell_list_first(uni_cell_list_t *plist) {
	return (plist
			? (const uni_cell_t*) plist->first
			: NULL) ;
}

/*----------------------------------------------------------------------*/

/* Get Last cell of list */
static inline const uni_cell_t * _uni_cell_list_last(uni_cell_list_t *plist) {
	return (plist
			? (const uni_cell_t*) plist->last
			: NULL) ;
}

/*----------------------------------------------------------------------*/

/* Extract First cell of list */
static uni_cell_t * _uni_cell_list_extract(uni_cell_list_t *plist) {
	uni_cell_t * lp_cell = NULL ;

	if (!plist || !plist->first) return NULL ;

	lp_cell = plist->first ;
	plist->first = lp_cell->next ;

	/* Case of single cell list */
	if (!plist->first)
		plist->last = NULL ;

	lp_cell->next = NULL ;
	plist->nbcells-- ;
	return lp_cell ;
}

/*----------------------------------------------------------------------*/

/* Insert a new cell at the beginning of the list */

static int _uni_cell_list_insert(uni_cell_list_t *plist, uni_cell_t *pcell) {
	if (!plist || !pcell) return -EINVAL ;

	/* Manage empty list case */
	if (!plist->first) {
		pcell->next	= NULL ;
		plist->first	= pcell ;
		plist->last	= pcell ;
	} else {
		pcell->next	= plist->first ;
		plist->first	= pcell ;
	}

	plist->nbcells++ ;
	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Add a new cell at the end of the list */

static int _uni_cell_list_append(uni_cell_list_t *plist, uni_cell_t *pcell) {
	if (!plist || !pcell) return -EINVAL ;

	pcell->next = NULL ;

	/* Manage empty list case */
	if (!plist->nbcells) {
		plist->first		= pcell ;
		plist->last		= pcell ;
	} else {
		plist->last->next	= pcell ;
		plist->last		= pcell ;
	}

	plist->nbcells++ ;
	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Concatanate 2 lists */
static int _uni_cell_list_cat(
		uni_cell_list_t *phead,
		uni_cell_list_t *ptail) {
	if (!phead || !ptail) return -EINVAL ;
	if (!ptail->nbcells) return 0 ;

	phead->nbcells += ptail->nbcells ;
	if (phead->last) {
		phead->last->next	= ptail->first ;
		phead->last		= ptail->last ;
	} else {
		phead->first		= ptail->first ;
		phead->last		= ptail->last ;
	}

	/* Reset Second List */
	ptail->nbcells	= 0 ;
	ptail->first	= NULL ;
	ptail->last	= NULL ;

	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Move cursor forward */

static inline int _uni_cell_list_crs_next(uni_cell_list_crs_t *pcursor) {
	if (!pcursor || !*pcursor) return -EINVAL ;
	*pcursor = (uni_cell_list_crs_t) ((uni_cell_t*)*pcursor)->next ;
	return 0 ;
}

/*----------------------------------------------------------------------
 *
 * Split raw AAL5 frame into UNI cells
 * We need to format the AAL5 trailer
 *
 */
static int _aal5_to_cells(
		int			vpi,	/* IN: VPI		*/
		int			vci,	/* IN: VCI		*/
		size_t			szaal5,	/* IN: AAL5 size	*/
		byte *			aal5,	/* IN: AAL5 raw data	*/
		uni_cell_list_t *		plist	/* IN/OUT: filled cells */
) {
	size_t		lv_szdata	= szaal5 ;
	byte *		lp_data		= aal5 ;
	size_t		lv_szpadd	= 0 ;
	byte		lv_aal5trl[ATM_CELL_PAYLOAD] ;
	u32		lv_crc32	= -1 ;
	int		lv_rc 		= 0 ;
	uni_cell_t *	lp_cell		= NULL ;
	uni_cell_list_t	lv_tmplist ;
	
	/* Check Interface */
	if (!aal5 || !plist) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	/* Check aal5 size */
	if (szaal5 > 0xffff) {
		ERR_OUT(
			"data too long to be hold in aal5 frame [%d/%d]\n",
			szaal5,
			0xffff) ;
		return -EINVAL ;
	}

	/* Init Temp list */
	lv_rc = _uni_cell_list_init(&lv_tmplist) ;
	if (lv_rc) {
		ERR_OUT("Failed to init temp list\n") ;
		return -1 ;
	}
	
	/*
	 * |------------------|---------|---------|--------|-----|
	 * |    User Data     |   PAD   | Control | Length | CRC |
	 * |------------------|---------|---------|--------|-----|
	 *        0-65535        0-47        2        2       4
	 *
	 * PAD : to make the full frame to fit 48 bytes boundery
	 * Control(UU + CPI) : Reserved (set to 0)
	 *
	 */

	/* Split AAL5 into UNI Cells */
	while (lv_szdata > ATM_CELL_PAYLOAD) {

		lp_cell = _uni_cell_alloc() ;
		if (!lp_cell) {
			_uni_cell_list_reset(&lv_tmplist) ;
			return -ENOMEM ;
		}
		
		/* Format slice */
		lv_rc = _uni_cell_format(
				vpi,
				vci,
				false,
				ATM_CELL_PAYLOAD,
				lp_data,
				lp_cell) ;
		if (lv_rc) {
			ERR_OUT("_uni_cell_format failed\n") ;
			_uni_cell_free(lp_cell) ;
			_uni_cell_list_reset(&lv_tmplist) ;
			return lv_rc ;
		}

		/* compute crc on the fly */
		lv_crc32 = _calc_crc(
				_uni_cell_getPayload(lp_cell),
				ATM_CELL_PAYLOAD,
				lv_crc32) ;

		/* Append slice to cell list */
		lv_rc = _uni_cell_list_append(&lv_tmplist, lp_cell) ;
		if (lv_rc) {
			_uni_cell_free(lp_cell) ;
			_uni_cell_list_reset(&lv_tmplist) ;
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

		lp_cell = _uni_cell_alloc() ;
		if (!lp_cell) {
			_uni_cell_list_reset(&lv_tmplist) ;
			return -ENOMEM ;
		}

		/* Format this before last slice */
		lv_rc = _uni_cell_format(
				vpi,
				vci,
				false,
				lv_szdata,
				lp_data,
				lp_cell) ;
		if (lv_rc) {
			ERR_OUT("_uni_cell_format failed on before last cell\n") ;
			_uni_cell_free(lp_cell) ;
			_uni_cell_list_reset(&lv_tmplist) ;
			return lv_rc ;
		}

		lp_data		+= lv_szdata ;
		lv_szdata	= 0 ;

		/* compute crc on the fly */
		lv_crc32 = _calc_crc(
				_uni_cell_getPayload(lp_cell),
				ATM_CELL_PAYLOAD,
				lv_crc32) ;

		/* Append slice to cell list */
		lv_rc = _uni_cell_list_append(&lv_tmplist, lp_cell) ;
		if (lv_rc) {
			_uni_cell_free(lp_cell) ;
			_uni_cell_list_reset(&lv_tmplist) ;
			return lv_rc ;
		}

	} else {
		/* copy remaining data in trailer cell */
		memcpy(lv_aal5trl, lp_data, lv_szdata) ;
	}

	/* 
	 * Format the trailer
	 */

	/* Compute size of padding */
	lv_szpadd = ATM_CELL_PAYLOAD - lv_szdata - ATM_AAL5_TRAILER ;
	
	/* Set the padding and reset Control */
	memset(
		lv_aal5trl + lv_szdata, 
		0, 
		lv_szpadd + 2) ;
	
	/* Set the length (no CPI & no UU) */
	lv_aal5trl[ATM_CELL_PAYLOAD - 6] = (szaal5 & 0xffff) >> 8 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 5] = (szaal5 & 0x00ff) ;

	/* finalize CRC computing */
	lv_crc32 = ~ _calc_crc(
			lv_aal5trl, 
			ATM_CELL_PAYLOAD - 4,
			lv_crc32) ;

	/* 
	 * Set the CRC 
	 */
	lv_aal5trl[ATM_CELL_PAYLOAD - 4] = lv_crc32 >> 24 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 3] = (lv_crc32 & 0x00ff0000) >> 16 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 2] = (lv_crc32 & 0x0000ff00) >> 8 ;
	lv_aal5trl[ATM_CELL_PAYLOAD - 1] = (lv_crc32 & 0x000000ff) ;

	/* Append the last Cell */
	lp_cell = _uni_cell_alloc() ;
	if (!lp_cell) {
		_uni_cell_list_reset(&lv_tmplist) ;
		return -ENOMEM ;
	}

	/* Format this last slice */
	lv_rc = _uni_cell_format(
			vpi,
			vci,
			true,
			ATM_CELL_PAYLOAD,
			lv_aal5trl,
			lp_cell) ;
	if (lv_rc) {
		ERR_OUT("_uni_cell_format failed on last cell\n") ;
		_uni_cell_free(lp_cell) ;
		_uni_cell_list_reset(&lv_tmplist) ;
		return lv_rc ;
	}

	/* Append slice to cell list */
	lv_rc = _uni_cell_list_append(&lv_tmplist, lp_cell) ;
	if (lv_rc) {
		_uni_cell_list_reset(&lv_tmplist) ;
		return lv_rc ;
	}

	/* Append the built list */
	lv_rc = _uni_cell_list_cat(plist, &lv_tmplist) ;
	if (lv_rc) {
		ERR_OUT("Could not return the built list\n") ;
	}

	_uni_cell_list_reset(&lv_tmplist) ;

	return lv_rc ;
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

	lp_aal5 = kmalloc(sizeof(aal5_t), GFP_ATOMIC) ;
	if (!lp_aal5) return NULL ;
	
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
static inline int _aal5_getVpi(
	aal5_t * paal5			/* IN: the frame		*/
) {
	if (!paal5 || !paal5->is_valid) return -1 ;
	return paal5->vpi ;
}

/*----------------------------------------------------------------------*/

/* Get VCI from AAL5 */
static inline int _aal5_getVci(
	aal5_t * paal5			/* IN: the frame		*/
) {
	if (!paal5 || !paal5->is_valid) return -1 ;
	return paal5->vci ;
}

/*----------------------------------------------------------------------*/

/* check if this is the frame is completed */
static inline bool _aal5_iscomplete(
	aal5_t * paal5			/* IN: the frame		*/
) {
	return (paal5 ? paal5->is_complete : false) ;
}	

/*----------------------------------------------------------------------*/

/* check if this is the frame is valid */
static inline bool _aal5_isvalid(
	aal5_t * paal5			/* IN: the frame		*/
) {
	return (paal5 && paal5->is_complete ? paal5->is_valid : false) ;
}

/*----------------------------------------------------------------------*/

/* Get AAL5 frame size */
static inline size_t _aal5_getSize(
	aal5_t * paal5			/* IN: the frame		*/
) {
	uni_cell_t * lp_last = NULL ;

	if (!paal5 || !_aal5_iscomplete(paal5)) return 0 ;

	lp_last = paal5->plastcell ;

	return ((lp_last->raw[ATM_CELL_SZ - 6] << 8) & 0xff00)
		+ lp_last->raw[ATM_CELL_SZ - 5] ;
}

/*----------------------------------------------------------------------*/

/* Get the number of cells in the AAL5 frame */
static inline int _aal5_getNbCell(
	aal5_t * paal5			/* IN: the frame		*/
) {
	return (paal5 ? paal5->nbcells : -1 ) ;
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
		paal5->_crc 	= _calc_crc(
					_uni_cell_getPayload(pcell),
					ATM_CELL_PAYLOAD,
					paal5->_crc) ;
	} else {
		paal5->is_complete = true ;
		
		/* compute final crc */
		paal5->_crc 	= ~ _calc_crc(
					pcell->raw + ATM_CELL_HDR,
					ATM_CELL_PAYLOAD - 4,
					paal5->_crc) ;

		/* get given length */
		lv_length = (pcell->raw[ATM_CELL_SZ - 6] << 8)
			+ (pcell->raw[ATM_CELL_SZ - 5]) ;
		lv_length = (lv_length + 8 + 
				ATM_CELL_PAYLOAD - 1) / ATM_CELL_PAYLOAD ;
		
		/* get given CRC */
		lv_crc =  ((pcell->raw[ATM_CELL_SZ - 4] << 24)	& 0xff000000)
			+ ((pcell->raw[ATM_CELL_SZ - 3] << 16)	& 0x00ff0000)
			+ ((pcell->raw[ATM_CELL_SZ - 2] << 8 )	& 0x0000ff00)
			+ ((pcell->raw[ATM_CELL_SZ - 1])	& 0x000000ff);

		/* compare to given crc */
		if (lv_crc != paal5->_crc) {
			ERR_OUT("Invalid crc field in trailer\n") ;
#ifdef DEBUG
			if (lv_length > (paal5->nbcells + 1)) {
				DBG_OUT("We have [%d/%d] cells,"
						"may miss data",
						paal5->nbcells + 1,
						lv_length) ;
			} else if (lv_length < (paal5->nbcells + 1)) {
				DBG_OUT("We have [%d/%d] cells,"
						"may have too much data",
						paal5->nbcells + 1,
						lv_length) ;
			}
#endif /* DEBUG */
			return -EINVAL ;
		}
		
		/* check given length */
		if (lv_length != (paal5->nbcells + 1)) {
			ERR_OUT("Invalid length field in trailer\n") ;
 			DBG_OUT(
 				"should have [%d] cells and have [%d]\n",
 				lv_length,
 				paal5->nbcells + 1) ;
			return -EINVAL ;
		}

		paal5->is_valid = true ;
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
