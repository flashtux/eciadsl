/*****************************************************************************
*                                                                            *
*     Driver pour le modem ADSL ECI HiFocus utilise par France Telecom       *
*                                                                            *
*     Author : Valette Jean-Sebastien <jeanseb.valette@free.fr>              *
*              Eric Bardes  <eric.bardes@wanadoo.fr>                         *
*              Benoit PAPILLAULT <benoit.papillault@free.fr>                 *
*                                                                            *
*     License : GPL                                                          *
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

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/version.h>

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
		lp_cur += sprintf(lp_cur, "%s %02x:", KERN_DEBUG, lc_1);
		
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

#define ECI_NB_ISO_PACKET   20 /* number of ISO URB */
#define ECI_ISO_PACKET_SIZE 448
#define ECI_ISO_PACKET_NB   10 /* number of packets in one URB */
#define ECI_ISO_BUFFER_SIZE (ECI_ISO_PACKET_SIZE * ECI_ISO_PACKET_NB)

#define ECI_NB_BULK_PKT 10
#define ECI_BULK_BUFFER_SIZE (64 * ECI_NB_BULK_PKT)

#define ECI_STATE_PROBING 0
#define ECI_STATE_SYNCHRONIZING 1
#define ECI_STATE_RUNNING 2
#define ECI_STATE_REMOVING 3

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

/*
  Format of the usb_packets.h (or usb_packets_dmt.h) file :

  It should define an array eci_init_setup_packets[] which the following
  contents:

*/

#ifndef _DMT_
  #include "usb_packets.h"
#else
  #include "usb_packets_dmt.h"
#endif

/**********************************************************************
                    USB stuff
***********************************************************************/

static const struct usb_device_id eci_usb_deviceids[] =
{
	{ USB_DEVICE ( 0x0915 , 0x8000 ) } , /* ECI HI FOCUS */
					    /* ECI B FOCUS  */
	{ USB_DEVICE ( 0x0915 , 0xac82 ) } ,  /* EICON DIVA USB */
	{}
};

#define ECI_NB_MODEMS 2
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
static void eci_init_vendor_callback(struct urb *urb);
static void eci_int_callback(struct urb *urb);
static void eci_iso_callback(struct urb *urb);
static void eci_bulk_callback(struct urb *urb);
#else
static void eci_init_vendor_callback(struct urb *urb, struct pt_regs *regs);
static void eci_int_callback(struct urb *urb, struct pt_regs *regs);
static void eci_iso_callback(struct urb *urb, struct pt_regs *regs);
static void eci_bulk_callback(struct urb *urb, struct pt_regs *regs);
#endif
static void eci_bh_iso(unsigned long instance);
static void _eci_send_init_urb(struct urb *eciurb);
static void eci_bh_bulk(unsigned long instance);





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
static void eci_bh_atm (unsigned long param);

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
static inline uni_cell_t * _uni_cell_fromRaw(size_t, byte*) ;

/* Free an UNI Cell 	*/
static inline void _uni_cell_free(uni_cell_t *) ;

/* Format a cell */
static inline int _uni_cell_format(
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
static inline void _uni_cell_setHEC(
	uni_cell_t *		/* in: the cell			*/
) ;

/* Check HEC of header */
static inline bool _uni_cell_chkHEC(
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
static inline int _uni_cell_qinit(uni_cell_q_t *) ;

/* Alloc a new queue */
static inline uni_cell_q_t * _uni_cell_qalloc(void) ;

/* Free a queue */
static inline void _uni_cell_qfree(uni_cell_q_t *) ;

/* Check if the queue is empty */
static inline bool _uni_cell_qisempty(uni_cell_q_t *) ; 

/* Push new UNI Cell in Queue */
static int _uni_cell_qpush(
	uni_cell_t *,		/* IN: Cell to insert	*/
	uni_cell_q_t *		/* IN: Target queue	*/
) ;

/* Pop an UNI Cell from Queue */
static inline uni_cell_t * _uni_cell_qpop(uni_cell_q_t *) ;

/* Get UNI Cell on head of Queue */
static inline uni_cell_t * _uni_cell_qhead(uni_cell_q_t *) ;

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
static inline int _uni_cell_list_init(uni_cell_list_t *) ;

/* Reset a list */
static inline void _uni_cell_list_reset(uni_cell_list_t *) ;

/* Alloc a new list */
static inline uni_cell_list_t * _uni_cell_list_alloc(void) ;

/* Free a list (free each cells) */
static inline void _uni_cell_list_free(uni_cell_list_t *) ;

/* Get number of Cells in list (<0 : error)*/
static inline int _uni_cell_list_nbcells(uni_cell_list_t *) ;

/* Get First cell of list */
static inline const uni_cell_t * _uni_cell_list_first(uni_cell_list_t *) ;

/* Get Last cell of list */
static inline const uni_cell_t * _uni_cell_list_last(uni_cell_list_t *) ;

/* Extract First cell of list */
static inline uni_cell_t * _uni_cell_list_extract(uni_cell_list_t *) ;

/* Insert a new cell at the beginning of the list */
static inline int _uni_cell_list_insert(uni_cell_list_t *, uni_cell_t *) ;

/* Add a new cell at the end of the list */
static inline int _uni_cell_list_append(uni_cell_list_t *, uni_cell_t *) ;

/* Concatenate 2 lists (reset second list) */
static inline int _uni_cell_list_cat(uni_cell_list_t*, uni_cell_list_t*) ;

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
static inline int _aal5_init(aal5_t *) ;
	
/* Alloc a new AAL5 frame */
static inline aal5_t * _aal5_alloc(void) ;

/* Free an AAL5 frame */
static inline void _aal5_free(aal5_t *) ;

/* Reset an AAL5 frame */
static inline void _aal5_clean(aal5_t *) ;

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
static inline int _aal5_add_cell(
	aal5_t *,		/* IN: the frame		*/
	uni_cell_t *		/* IN: the new cell		*/
) ;

/* Get Next Cell of the AAL5 frame */
static inline uni_cell_t * _aal5_get_next(aal5_t *) ;


/* -- Interface -- */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
static int eci_atm_open(struct atm_vcc *vcc, short vpi, int vci);
#else
static int eci_atm_open(struct atm_vcc *vcc);
#endif
static void eci_atm_close(struct atm_vcc *vcc);
static int eci_atm_ioctl(struct atm_dev *dev,unsigned int cmd, void *arg);
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb);

/* -- Private Functions -- */
static u32 _calc_crc(unsigned char *, int, unsigned int) ;
static int _eci_tx_aal5( 
		int, int, 
		struct eci_instance *, struct sk_buff *
) ;
static int _eci_rx_aal5(struct eci_instance*, aal5_t*, struct atm_vcc *);

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
	owner:	THIS_MODULE,
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
static void __exit eci_exit(void);

MODULE_AUTHOR( "Eric Bardes, Jean-Sebastien Valette" );
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION( "ECI HiFocus ADSL Modem Driver");
module_init (eci_init);
module_exit (eci_exit);


#endif
/**********************************************************************
                    USB Driver stuff
***********************************************************************/

/*	prototypes	*/	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void *eci_usb_probe(struct usb_device *dev,unsigned int ifnum , 
		const struct usb_device_id *id);
#else
static int eci_usb_probe(struct usb_interface *interface, 
		const struct usb_device_id *id);
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void eci_usb_disconnect(struct usb_device *dev, void *p);
#else
static void eci_usb_disconnect(struct usb_interface *interface);
#endif
int eci_usb_ioctl(struct usb_device *usb_dev,unsigned int code, void * buf);
static int eci_usb_send_urb(struct eci_instance *instance, 
			struct uni_cell_list *cells);


/*	structures	*/
#define NBMAXCODES 20 /* int code buffer size */

struct eci_instance 		/*	Private data for driver	*/
{
	struct eci_instance	*next;	
	spinlock_t		lock;		/*	lock for the struct */
	struct usb_device 	*usb_wan_dev;
	struct atm_dev		*atm_dev;
	int 			minor;
	int state;	/* Device state */
	wait_queue_head_t 	eci_wait;	/*	no more used	*/
	const unsigned char	*setup_packets; /* 	For init	*/
	struct urb		*isourbs[ECI_NB_ISO_PACKET];
						/*	For Iso transfer */
	struct urb		*vendor_urb;	/*	For init and Answer 
							to INT		*/
	struct urb		*interrupt_urb;	/*	INT pooling	*/
	struct urb		*bulk_urb;	/*	outgoing datas	*/
	int			bulkisfree;	/*	Teel if bulk urb 
							available	*/
	unsigned char 		*interrupt_buffer;
	unsigned char 		*pooling_buffer;
	char			iso_celbuf[ATM_CELL_SZ]; /* incomplete cell */
	int			iso_celbuf_pos; /*	Pos in cell buf	    */
	struct tasklet_struct	bh_iso;		/*	incoming datas	BH  */
 	struct uni_cell_list	iso_cells;	/* 	incoming cell Q     */
	struct tasklet_struct	bh_bulk;	/*	outgoing urb BH     */
 	struct uni_cell_list	bulk_cells;	/* 	outgoing urb Q      */
	struct tasklet_struct	bh_atm;		/*	outgoing datasBH    */
	struct sk_buff_head	txq;		/*	outgoing datas Q    */
	u16			match73,rep73;	/*	interrupt stuf	    */
	u16			match53,rep53;	/*	byte for replacement*/	
	int			ctrlcodecnt;	/*	control code counter*/
	int			ctrlseqcnt;	/*	sequence number     */
	int			cnt734d;	/*	specific code counter*/
	int			cnt7341;	/*	specific code counter*/
	int			cnt53;		/*	global code counter */
	int			cnt73;		/*	global code counter */
	u16			ctrlcodes[NBMAXCODES]; /* ctrl code buffer  */
	
	
	
	/* -- test -- */
	struct atm_vcc		*pcurvcc ;
 	struct aal5		*pbklogaal5;	/*	AAL5 to complete */
};

struct usb_driver eci_usb_driver = {
	owner:  THIS_MODULE,
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


static int __init eci_init(void) {	
	int lv_res = 0 ;

	/*	Must register driver and claim for hardware */

	printk("ECI HiFocus ADSL Modem Driver  loading\n");
	DBG_OUT("$Id$\n");
	lv_res = usb_register(&eci_usb_driver) ;

	return lv_res ;

}

/*
		free all allocated ressource for this instance
*/
static int _eci_cleanup_instance(struct eci_instance *i) {
	int cnt;
	i->state = ECI_STATE_REMOVING;
	ERR_OUT("CLEANUP 1\n");
#ifdef __USE_ATM__
	if(i->atm_dev) 	{
		shutdown_atm_dev(i->atm_dev);
		i->atm_dev = 0;
	}
#endif	
	
	DBG_OUT("CLEANUP 2\n");
	for(cnt = 0 ; cnt< ECI_NB_ISO_PACKET;cnt ++) 
			if(i->isourbs[cnt]){
				if (i->isourbs[cnt]->status == -EINPROGRESS)
					usb_unlink_urb(i->isourbs[cnt]);
				usb_free_urb(i->isourbs[cnt]);
				i->isourbs[cnt] = 0;
			}
	DBG_OUT("CLEANUP 3\n");
	if(i->vendor_urb) 	{
		if (i->vendor_urb->status == -EINPROGRESS)
			usb_unlink_urb(i->vendor_urb);
		usb_free_urb(i->vendor_urb);
		i->vendor_urb = 0;
	}
	DBG_OUT("CLEANUP 4\n");
	if(i->interrupt_urb){
		if (i->interrupt_urb->status == -EINPROGRESS)
			usb_unlink_urb(i->interrupt_urb);
		usb_free_urb(i->interrupt_urb);
		i->interrupt_urb = 0;
	}
	DBG_OUT("CLEANUP 5\n");
	if(i->bulk_urb) {
		if (i->bulk_urb->status == -EINPROGRESS)
			usb_unlink_urb(i->bulk_urb);
		usb_free_urb(i->bulk_urb);
		i->bulk_urb = 0;
	}
	DBG_OUT("CLEANUP 6\n");
	if(i->bh_bulk.func) {
		tasklet_kill(&i->bh_bulk);
		i->bh_bulk.func = 0;
	}
	DBG_OUT("CLEANUP 7\n");
	if(i->bh_iso.func) {
		tasklet_kill(&i->bh_iso);
		i->bh_iso.func = 0;
	}
	DBG_OUT("CLEANUP 8\n");
	if(i->bh_atm.func) {
		tasklet_kill(&i->bh_atm);
		i->bh_atm.func = 0;
	}
	DBG_OUT("CLEANUP 9\n");
	_uni_cell_list_reset(&i->iso_cells);
	_uni_cell_list_reset(&i->bulk_cells);
	/*skb_free(i->txq);*/
	DBG_OUT("CLEANUP 10\n");
	if(i->pcurvcc) 	{
	}
	if(i->pbklogaal5) 	{
		_aal5_free(i->pbklogaal5);
		i->pbklogaal5 = 0;
	}
	DBG_OUT("CLEANUP 11\n");
	return 0;
}

static void __exit eci_exit(void) {
	uni_cell_t * lp_cell = gp_ucells ;
		
	usb_deregister(&eci_usb_driver);

	/* Free Unused UNI Cell List */
	while (lp_cell) {
			gp_ucells = lp_cell->next ;
			kfree(lp_cell) ;
			lp_cell = gp_ucells ;
	}

	return;
};
	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void *eci_usb_probe(struct usb_device *dev,unsigned int ifnum , 
	const struct usb_device_id *id) {
#else
static int eci_usb_probe(struct usb_interface *interface, 
	const struct usb_device_id *id) {
	struct usb_device *dev= interface_to_usbdev(interface);	
#endif 
	struct eci_instance *out_instance= NULL;
	struct urb *eciurb;
	/* unused : int dir; */
	int eci_isourbcnt;
	int i;	/*	loop counter	*/
	int pid=0, vid=0; /* for device checking */


/*		We don't yet support more than one modem connected */
	
/************************************************

	The code Here will have to initialize modem 
	then claim for wan usb device
	
************************************************/
	for(i=0;i<ECI_NB_MODEMS;i++) 	{
		if(eci_usb_deviceids[i].idVendor == dev->descriptor.idVendor
			&& eci_usb_deviceids[i].idProduct == 
				dev->descriptor.idProduct) 		{
					pid = eci_usb_deviceids[i].idProduct;
					vid = eci_usb_deviceids[i].idVendor;
					break;
		}
	}

	if(dev->descriptor.bDeviceClass == USB_CLASS_VENDOR_SPEC &&
	   dev->descriptor.idVendor == vid &&
	   dev->descriptor.idProduct == pid )	{
			out_instance = kmalloc(sizeof(struct eci_instance), 
							GFP_KERNEL);
			if(out_instance) {
				memset(out_instance, 0, sizeof(struct eci_instance));
				spin_lock_init(&out_instance->lock);
				out_instance->state = ECI_STATE_PROBING;
				out_instance->usb_wan_dev= dev;
				out_instance->setup_packets = eci_init_setup_packets;
				out_instance->iso_celbuf_pos = 0 ;
				out_instance->bh_iso.func = eci_bh_iso ;
				out_instance->bh_iso.data = (unsigned long)  out_instance ;
			 	if(_uni_cell_list_init(&out_instance->iso_cells)) {
					ERR_OUT("Cant init Bulk list\n");
					goto erreure;
				}
				out_instance->bh_bulk.func = eci_bh_bulk;
				out_instance->bh_bulk.data = (unsigned long) out_instance ;
			 	if(_uni_cell_list_init(&out_instance->bulk_cells)) {
					ERR_OUT("Cant init Bulk list\n");
					goto erreure;
				}
			} else {
				ERR_OUT("out of memory\n");
				goto erreure;
			}
/*
			if(usb_set_configuration(dev,1)<0) {
				ERR_OUT("Can't set configuration\n");
				goto erreure;
			}
*/
			if(usb_set_interface(dev,0,4)<0) {
				ERR_OUT("Cant set interface\n");
				goto erreure;
			}
		/*
			send 20 Urbs to the iso EP
		*/
		for(eci_isourbcnt=0;eci_isourbcnt<ECI_NB_ISO_PACKET;eci_isourbcnt++)
      {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))			
				if(!(eciurb = usb_alloc_urb(ECI_ISO_PACKET_NB))) {
#else
				if(!(eciurb = usb_alloc_urb(ECI_ISO_PACKET_NB, GFP_KERNEL))) {
#endif
					ERR_OUT(	"not enought memory for iso urb %d\n", eci_isourbcnt);
					goto erreure;
				}
			if(!(eciurb->transfer_buffer=kmalloc(	ECI_ISO_BUFFER_SIZE, 
				GFP_KERNEL))) {
					ERR_OUT(	"not enought memory for iso buffer %d\n", eci_isourbcnt);
					goto erreure;
			}
			eciurb->dev = dev;
			eciurb->context = out_instance;
			eciurb->pipe = usb_rcvisocpipe(dev, ECI_ISO_PIPE);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
			eciurb->transfer_flags = USB_ISO_ASAP;
#else
			eciurb->transfer_flags = URB_ISO_ASAP;
			eciurb->interval = 1;
#endif
      eciurb->start_frame       = 0;
			eciurb->number_of_packets = ECI_ISO_PACKET_NB;
			eciurb->complete = eci_iso_callback;
			eciurb->transfer_buffer_length = ECI_ISO_BUFFER_SIZE; 
			for(i=0;i < ECI_ISO_PACKET_NB; i ++) {
				eciurb->iso_frame_desc[i].offset = 	i * ECI_ISO_PACKET_SIZE;
				eciurb->iso_frame_desc[i].length =      ECI_ISO_PACKET_SIZE;
				eciurb->iso_frame_desc[i].actual_length = 0 ;
        eciurb->iso_frame_desc[i].status = 0;
			}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))			
			if(usb_submit_urb(eciurb)) {
#else
			if(usb_submit_urb(eciurb, GFP_KERNEL)) {
#endif			
				usb_free_urb(eciurb);
				ERR_OUT("error couldn't send iso urbs %d\n", eci_isourbcnt);
				goto erreure;
			}
      DBG_OUT("ISO URB %2d sent\n",eci_isourbcnt);
			out_instance->isourbs[eci_isourbcnt] = eciurb;
		}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))		
		if(!(eciurb=usb_alloc_urb(0))) {
#else
		if(!(eciurb = usb_alloc_urb(0, GFP_KERNEL))) {
#endif		
			ERR_OUT("Can't allocate int urb !\n");
			goto erreure;
		}
	/*	interval tacken from cat /proc/bus/usb/devices values	*/
		out_instance->interrupt_urb = eciurb;
		if(!(out_instance->interrupt_buffer=kmalloc(64,GFP_KERNEL))) {
			ERR_OUT("Can't allocate int buffer\n");
			goto erreure;
		}
		if(!(out_instance->pooling_buffer = kmalloc(34+8, GFP_KERNEL))) {
			ERR_OUT("Can't alloc buffer for interrupt polling\n");
			goto erreure;
		}
		eciurb->setup_packet = eciurb->transfer_buffer  +34;	
		DBG_OUT("interrupt buffer = %p\n", out_instance->interrupt_buffer);
		usb_fill_int_urb(eciurb, dev, usb_rcvintpipe(dev, ECI_INT_EP), 
			out_instance->interrupt_buffer,64,
			eci_int_callback,out_instance,3);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))			
		if(usb_submit_urb(eciurb)) {
#else
		if(usb_submit_urb(eciurb, GFP_KERNEL)) {
#endif		
			ERR_OUT("error couldn't send interrupt urb\n");
			goto erreure;
		}
		/*
			Allocate bulk urb	
		*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		if(!(out_instance->bulk_urb = usb_alloc_urb(0))) {
#else
		if(!(out_instance->bulk_urb = usb_alloc_urb(0, GFP_KERNEL))) {
#endif
			ERR_OUT("Can't alloacate bulk URB\n");
			goto erreure;
		}
		/*
			Allocate bulk urb transfert buffer
		*/
		if(!(out_instance->bulk_urb->transfer_buffer=kmalloc(
				2*ECI_ISO_BUFFER_SIZE, GFP_KERNEL))) {
					ERR_OUT("not enought memory for bulk buffer\n");
					goto erreure;
		}
		/*	
			Now bulk URB AVAILABLE
		*/
		out_instance->bulkisfree = 1;
		/*
			Just Reset EP 0x02 (no reset)
			if some one day, all endpoint before sending any urb
			Send vendors URB 
			First allocate the urb struct
		*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		eciurb=usb_alloc_urb(0);
#else
		eciurb=usb_alloc_urb(0, GFP_KERNEL);
#endif
		if(!eciurb) {
			ERR_OUT("Can't allocate urb\n");
			goto erreure;
		};
		memset(eciurb, 0, sizeof(struct urb));
		/*
			Next allocate buffer for data transmition.
			Last 8 byte will be reserved for setupackets
		*/
		if(!(eciurb->transfer_buffer = kmalloc((64*1024), GFP_KERNEL))) {
			ERR_OUT(" out of memory\n");
			goto erreure;
		};
		/*
			Initialise all urb struct
		*/
		eciurb->context = out_instance;
		out_instance->vendor_urb = eciurb;
		out_instance->state = ECI_STATE_SYNCHRONIZING;
		_eci_send_init_urb(eciurb);
	} else {
		ERR_OUT("Probleme finding modem !\n");
		DBG_OUT("vid,pid  = %02x,%02x, should be %02x,%02x\n",
			vid,pid ,
	   		dev->descriptor.idVendor, 
	   		dev->descriptor.idProduct);
		goto erreure;
	}
	
#ifdef __USE_ATM__

	out_instance->bh_atm.func	= eci_bh_atm ;
	out_instance->bh_atm.data	= (unsigned long) out_instance ;
	skb_queue_head_init (&out_instance->txq) ;


	out_instance->pcurvcc 		= NULL ;
	out_instance->pbklogaal5	= NULL ;

	out_instance->atm_dev = atm_dev_register(eci_drv_label,	&eci_atm_devops,
			-1,NULL) ;
	if(out_instance->atm_dev)	{
		out_instance->atm_dev->dev_data = (void *)out_instance;
		out_instance->atm_dev->ci_range.vpi_bits = ATM_CI_MAX;
		out_instance->atm_dev->ci_range.vci_bits = ATM_CI_MAX;
		out_instance->atm_dev->link_rate = 640*1024/8/53;
/*	Further ATM DEVICE Should be initialize here	*/
	} else {
		ERR_OUT("Can't init ATM device\n");
		goto erreure;
	}
#endif /* __USE_ATM__ */
	
	DBG_OUT("out_instance : %p\n", out_instance);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
	return out_instance;
#else
	usb_set_intfdata(interface, out_instance);
	return 0;
#endif
erreure:
	if(out_instance) 	{
		_eci_cleanup_instance(out_instance);
		kfree(out_instance);
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
	return 0;
#else
	return -ENOMEM;
#endif
};

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void eci_usb_disconnect(struct usb_device *dev, void *p) {
	struct eci_instance *eciInstance = p;
#else
static void eci_usb_disconnect(struct usb_interface *interface) {
	struct eci_instance *eciInstance;
	
	eciInstance = (struct eci_instance*) usb_get_intfdata(interface);
#endif

	if(eciInstance) {

		_eci_cleanup_instance(eciInstance);
		kfree(eciInstance);
#if(! (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)))
		usb_set_intfdata (interface, NULL);
#endif
	}
};

int eci_usb_ioctl(struct usb_device *usb_dev,unsigned int code, void * buf) {
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
static int eci_atm_open(struct atm_vcc *vcc, short vpi, int vci) {
#else
static int eci_atm_open(struct atm_vcc *vcc) {
	int vpi = vcc->vpi;
	int vci = vcc->vci;
#endif

  int lv_rc;
	
	DBG_OUT("eci_atm_open: vpi:%d vci:%d\n", vpi, vci);

#if defined(ATM_VPI_UNSPEC) && defined(ATM_VCI_UNSPEC)
	/* Check that VCI & VPI are specified */
	if (vpi == ATM_VPI_UNSPEC || vci == ATM_VCI_UNSPEC) {
    		WRN_OUT("rejecting open with unspecified VPI/VCI\n");
		return -EINVAL;
	}
#endif /* ATM_VPI_UNSPEC && ATM_VCI_UNSPEC */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23))	
	/*
	 * Check that a context don't exists already
	 */
	lv_rc = atm_find_ci(vcc, &vpi, &vci) ;
	if (lv_rc != 0) {
		ERR_OUT("atm_find_ci failed [%d]", lv_rc) ;
		return lv_rc ;
	}	
	DBG_OUT("params after find : [%hd.%d]\n", vpi, vci) ;
#endif
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
	/*	
	 * TODO: allow more than one VCC.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23))	
	((struct eci_instance*)vcc->dev->dev_data)->atm_dev->vccs = vcc;
	((struct eci_instance*)vcc->dev->dev_data)->atm_dev->last = vcc;	
#else
#endif

  DBG_OUT("eci_atm_open: OK\n");
	return 0;
};

/*----------------------------------------------------------------------*/
static void eci_atm_close(struct atm_vcc *vcc) {
	struct eci_instance * lp_instance = 
		(struct eci_instance*)vcc->dev->dev_data ;

  DBG_OUT("eci_atm_close: vpi:%d vci:%d\n",vcc->vpi, vcc->vci);

	lp_instance->pcurvcc = NULL ;
	/*
	 * 	TO DO allow to open more than one vcc
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23))	
	lp_instance->atm_dev->vccs = NULL;
	lp_instance->atm_dev->last = NULL;
#endif	
	if (lp_instance->pbklogaal5) {
		_aal5_free(lp_instance->pbklogaal5) ;
		lp_instance->pbklogaal5 = NULL ;
	}
};

/*----------------------------------------------------------------------*/
static int eci_atm_ioctl(struct atm_dev *dev,unsigned int cmd, void *arg)
{
	int lv_res = -EINVAL ;

  DBG_OUT("eci_atm_ioctl: cmd=0x%x arg=%p\n",cmd,arg);

	switch (cmd) {
		case ATM_QUERYLOOP:
			lv_res = put_user(ATM_LM_NONE, (int*)arg) ? -EFAULT : 0;
			break ;
		default:
			DBG_OUT("not supported IOCTL[%u]\n", cmd) ;
			lv_res =  -ENOIOCTLCMD;
			break ;
	}
	return lv_res ;
};

/*----------------------------------------------------------------------*/
static int eci_atm_send(struct atm_vcc *vcc, struct sk_buff *skb) {
	struct eci_instance *	lp_outinst	= NULL ;
	int			lv_rc		= 0 ;

/*
  DBG_OUT("eci_atm_send: vpi:%d vci:%d\n",vcc->vpi, vcc->vci);
*/
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
	ATM_SKB(skb)->vcc = vcc;
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
/*
		DBG_OUT("queue AAL5 data for bh_atm\n") ;
*/
    skb_queue_tail (&lp_outinst->txq, skb);
    tasklet_schedule (&lp_outinst->bh_atm);
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
		/* Free Socket Buffer */
		FREE_SKB(vcc, skb) ;
		break ;
	}
	return lv_rc ;
};

/*----------------------------------------------------------------------*/

/* -- Private Functions ------------------------------------------------*/
static void eci_bh_atm (unsigned long param)
{
	struct eci_instance * lp_instance = (struct eci_instance *)param ;
	struct sk_buff *lp_skb = NULL ;
	struct atm_vcc *lp_vcc = NULL ;
	int lv_rc;
	int flags;

/*
  DBG_OUT("eci_bh_atm: param:%d\n",param);
*/
	if(lp_instance->state == ECI_STATE_REMOVING) return;
	spin_lock_irqsave(&lp_instance->lock, flags);
	/*
	 * 	Allow  more than one VCC
	 * 	Probleme with eci_tx_all5 param, gotta change it to struct atm_vcc *
	 * 	in order thow allow choose one couple VPI/VCI between all opened vcc
	 */

	if((lp_skb = skb_dequeue(&lp_instance->txq)))
    {
/*
      DBG_OUT("skb_dequeue\n");
*/
      lp_vcc = ATM_SKB(lp_skb)->vcc;
		lv_rc = _eci_tx_aal5(lp_vcc->vpi, lp_vcc->vci, lp_instance, lp_skb) ;
		if (lv_rc) {
			ERR_OUT("_eci_tx_aal5 failed\n") ;
			atomic_inc(&lp_vcc->stats->tx_err) ;
		} else {
			atomic_inc(&lp_vcc->stats->tx) ;
		}
		/* Free Socket Buffer */
		FREE_SKB(ATM_SKB(lp_skb)->vcc, lp_skb);
	}
	spin_unlock_irqrestore(&lp_instance->lock, flags);
}
/*----------------------------------------------------------------------*/


#endif /* __USE_ATM__ */

/*******************************************************************************

		USB functions

*******************************************************************************/

/*
  _eci_send_init_urb: send the next CTRL URB from the list in
  setup_packets. The last URB is followed by a single zero byte
*/

static void _eci_send_init_urb(struct urb *eciurb)
{
	struct eci_instance *instance;
	unsigned char *setuppacket;
	unsigned int pipe;
  static int n = 0; /* block number */

  /* URB setup fields */
  unsigned char  request_type;
  unsigned char  request;
  unsigned short value;
  unsigned short index;
  unsigned short size;

	instance = eciurb->context;
	if(instance->state == ECI_STATE_REMOVING) return;		
	if(instance->setup_packets[0] != 0)
  {
    /* where 64 * 1024 - 8 comes from? */
		setuppacket = eciurb->transfer_buffer +  64 * 1024 -8;
		memcpy(setuppacket, instance->setup_packets,8);

    /* increments the block number */
    n++;

    /* values in setup_packets are in little endian (USB standards) */
    request_type = instance->setup_packets[0];
    request      = instance->setup_packets[1];
    value        = instance->setup_packets[2]|(instance->setup_packets[3]<<8);
    index        = instance->setup_packets[4]|(instance->setup_packets[5]<<8);
    size         = instance->setup_packets[6]|(instance->setup_packets[7]<<8);

    DBG_OUT("Block %3d: request_type=%02x request=%02x value=%04x index=%04x size=%04x\n",
            n, request_type, request, value, index, size);

			/*
				If write URB then read the buffer and
				set endpoint else just set enpoint
			*/

		if(request_type & 0x80) {
			pipe = usb_rcvctrlpipe(instance->usb_wan_dev,0);
		} else {
			memcpy(eciurb->transfer_buffer, instance->setup_packets+8,size);
			pipe = usb_sndctrlpipe(instance->usb_wan_dev,0); 
		}
		usb_fill_control_urb(eciurb, instance->usb_wan_dev, pipe, 
			setuppacket, eciurb->transfer_buffer, size, eci_init_vendor_callback,
			instance);
		if(instance->state != ECI_STATE_REMOVING)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))			
			if(usb_submit_urb(eciurb)) {
#else
			if(usb_submit_urb(eciurb, GFP_ATOMIC)) {
#endif
				ERR_OUT("error couldn't send init urb\n");
				return;
			}
			instance->setup_packets += 
				(request_type & 0x80) ? 8 : 8 + size;
		} else {	/*	gotta free urb and buffer !	*/
			kfree(eciurb->transfer_buffer);
			eciurb->transfer_buffer = 0;
			instance->state = ECI_STATE_RUNNING;
		}
}
/*
		This one is called to handle the modem sync.

		It must send the saved URB and checked for returns value

		Based on ECI-LOAD2 by benoit papillault

		Valette Jean-Sebastien 

		30 11 2001

*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void eci_init_vendor_callback(struct urb *urb) {
#else
static void eci_init_vendor_callback(struct urb *urb, struct pt_regs *regs) {
#endif
	struct eci_instance *instance;
	int size;

	instance = (struct eci_instance *) urb->context;
	spin_lock(&instance->lock);
/*
	If urb status is set warn about it, else do what we gotta do
*/
	if(urb->status)
		ERR_OUT("Urb with status %d\n" , urb->status);
	else {
		/*	Check For datarecived and wanted data length */
		if( urb->actual_length != urb->transfer_buffer_length )
			WRN_OUT( "expecting %d, got %d\n", urb->transfer_buffer_length, 
				urb->actual_length) ;
#ifdef DEBUG
		if ((urb->setup_packet[0] & 0x80)) {
			size = ((instance->setup_packets[7]<<8) | 
					instance->setup_packets[6]);
		}
#endif	/*	DEBUG	*/
		if ( !((urb->setup_packet[0] & 0x40) == 0x40 && urb->setup_packet[1]== 0xdc
			&&((urb->setup_packet[3] * 256 + urb->setup_packet[2])== 0x00) && 
			((urb->setup_packet[5] * 25 + urb->setup_packet[4]) == 0x00))) {
			/*	
				sould send next VENDOR URB
			*/
			_eci_send_init_urb(instance->vendor_urb);
		}
	}
	spin_unlock(&instance->lock);
}

/*
	Null call back for control urbs
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
static void eci_control_callback(struct urb *urb) {
#else
static void eci_control_callback(struct urb *urb, struct pt_regs *regs) {
#endif
	kfree(urb->context);
	return;
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void eci_int_callback(struct urb *urb) {
#else
static void eci_int_callback(struct urb *urb, struct pt_regs *regs) {
#endif
	struct eci_instance *instance;
	int i; 		/*loop counter	*/
	int outi = 0; 
	unsigned char *in_buf, b1, b2;
	static int eci_int_count = 0;
	static unsigned char replace_b1[] = { 0x73, 0x73, 0x63, 0x63,
		0x63, 0x63, 0x73, 0x73, 0x63, 0x63, 0x63 , 0x63 };
	static unsigned char replace_b2[] = { 0x11, 0x11, 0x13, 0x13,
		0x13, 0x13, 0x11, 0x11, 0x1, 0x1, 0x1, 0x01 };
	u16	ctrlcode;
	unsigned char eci_outbuf[34] = {
		0xff, 0xff, 
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc,
		0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc
	}; 
	static struct usb_ctrlrequest *dr;

/*
  DBG_OUT("eci_int_callback: urb:%p urb->actual_length:%d\n",
          urb, urb->actual_length);

  if (urb->actual_length != 0) {
    DBG_RAW_OUT("INT URB", urb->transfer_buffer, urb->actual_length);
  }
*/
	instance = (struct eci_instance *) urb->context;
	spin_lock(&instance->lock);

  /* if we are synchronizing, send the next vendor URB if packet
     length is 16 */
  if (urb->actual_length == 16)
    _eci_send_init_urb(instance->vendor_urb);
      

	if(urb->actual_length == 64)
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
			for(i=0;i<15;i++) 	{
				ctrlcode = (in_buf[6+2*i+0]<<8) | in_buf[6+2*i+1];
				switch(ctrlcode) {
						case 0x0c0c:
							break;
						case 0x7311:
							if(instance->cnt73 == 5 ) {
								  instance->cnt73 = 0;
					 		if(instance->ctrlcodecnt == 1 &&
					    		instance->ctrlcodes[1] == 0x7341)
								      if(++instance->cnt7341 == 3)
										   instance->rep73 = 0x730f;
						}
						case 0x5311:
							if(instance->cnt53 == 5 ) {
							  instance->cnt53 = 0;
							  if(instance->ctrlcodecnt == 1 &&
								    instance->ctrlcodes[1] == 0x7341 &&
					    			instance->rep53 == 0xc339)
					 				instance->rep53 = 0x4301;
							}
						default:
				  			if(instance->ctrlcodecnt >= 0) {
								if(instance->ctrlcodes[instance->ctrlcodecnt] != ctrlcode) {
								/*	Too Many code , problem */
									  if(instance->ctrlcodecnt == NBMAXCODES)
									  		break;
									  if(ctrlcode == 0xf301) /* RESET */
											instance->ctrlcodecnt = 0;
									  else {
										    instance->ctrlcodecnt++;
										    instance->ctrlcodes[instance->ctrlcodecnt] = ctrlcode;
										    instance->ctrlseqcnt = 1;
					  				}
								} else
									instance->ctrlseqcnt++;
				   			} else {
								instance->ctrlcodecnt++;
								instance->ctrlcodes[instance->ctrlcodecnt] = ctrlcode;
								instance->ctrlseqcnt = 1;
						   }
						   if(instance->ctrlseqcnt == 1) {
								switch(instance->ctrlcodecnt) {
									case 0:
										instance->rep53 = 0x4301;
										instance->rep73 = 0x6301;
										break;
					  				case 1:
										switch(instance->ctrlcodes[0]) {
										  case 0x734d:
											    if(instance->cnt734d)
											    	instance->rep73 = 0xe30b;
												else {
													instance->rep73 =	0x630b;
													instance->cnt734d = 0;
												}
										break;
										case 0x7431:
										    instance->cnt7341 = 0;
										    instance->rep73 = 0x6301;
										    instance->cnt73 = 0;
										    instance->rep53 = 0xc339;
										    instance->cnt53 = 0;
										default: /* Win a line ;) */
										    break;
										}
					  				case 2:
										switch(instance->ctrlcodes[2]) {
											case 0xf34f:
											    instance->rep73 = 0xe351;
												default: /* Win a line */
													break;
										}
							 	}
				    }
				}
			}
			for(i=0;i<15;i++)	{
				b1 = in_buf[6 + 2 * i + 0];  
				b2 = in_buf[6 + 2 * i + 1];

				if(b1 != 0x0c || b2 !=0x0c)	{
					switch(b1)	{
					  case 0x53:
					    if( ((b1 <<8 ) | b2 ) ==  instance->match53) {
							eci_outbuf[10 + outi] = instance->rep53 >> 8;
							eci_outbuf[10 + outi+1] = instance->rep53 & 0xf;
					    }
					    break;
					  case 0x73:
					    if( ((b1 <<8 ) | b2 ) == instance->match73)  {
							eci_outbuf[10 + outi] =  instance->rep73 >> 8;
							eci_outbuf[10 + outi+1] = instance->rep73 & 0xf;
					    }
					    break;
					  default: 
					    break;
					}
					outi+=2;
				}
			}
			if(outi)	{
/*				usb_control_msg(instance->usb_wan_dev, 
					usb_pipecontrol(0), 0xdd, 0x40, 0xc02,
				 	0x580 , eci_outbuf, 
					sizeof(eci_outbuf), HZ);*/
				dr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC);
				if(dr) {
					dr->bRequestType = 0x40;
					dr->bRequest = 0xdd;
					dr->wValue = cpu_to_le16(0xc02);
					dr->wIndex = cpu_to_le16(0x580);
					dr->wLength = cpu_to_le16(sizeof(eci_outbuf));
					usb_fill_control_urb(instance->vendor_urb,
						instance->usb_wan_dev, usb_pipecontrol(0),
						(unsigned char*) &dr,	eci_outbuf,
						sizeof(eci_outbuf),	eci_control_callback,dr);
					if(instance->state != ECI_STATE_REMOVING)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))						
						usb_submit_urb(instance->vendor_urb);
#else
						usb_submit_urb(instance->vendor_urb, GFP_ATOMIC);
#endif
				}
			}
  }

#if (!(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)))
	usb_fill_int_urb(urb, instance->usb_wan_dev, usb_rcvintpipe(instance->usb_wan_dev, ECI_INT_EP), 
			instance->interrupt_buffer,64,
			eci_int_callback,instance,3);
		if(usb_submit_urb(urb, GFP_ATOMIC)) {
			ERR_OUT("error couldn't send interrupt urb in int callback\n");
		}
#endif
	spin_unlock(&instance->lock);
}

static void eci_bh_iso(unsigned long instance)
{
	int flags;

/*
  DBG_OUT("eci_bh_iso\n");
*/

	if(((struct eci_instance *)instance)->state == ECI_STATE_REMOVING) return;
	spin_lock_irqsave(&(((struct eci_instance *)instance)->lock), flags);
 	eci_atm_receive_cell(((struct eci_instance *)instance),
		&(((struct eci_instance *)instance)->iso_cells));
	spin_unlock_irqrestore(&(((struct eci_instance *)instance)->lock), flags);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))	
static void eci_iso_callback(struct urb *urb) {
#else
static void eci_iso_callback(struct urb *urb, struct pt_regs *regs) {
#endif
	struct eci_instance 	*instance;	/* Driver private structre */
	int 			i;		/* Frame Counter	*/
 	int 			pos;		/* Buffer curent pos counter */
	struct uni_cell 	*cell;		/* Working cell		*/
	unsigned char 		*buf;		/* Working buffer pointer */
	int 			received = 0;	/* boolean for cell in frames */

/*
  if (urb->actual_length > 0 || urb->status != 0) {
    DBG_OUT("eci_iso_callback: urb=%p urb->status=%d urb->actual_length:%d\n",
            urb, urb->status, urb->actual_length);
  }
*/

	instance = (struct eci_instance *)urb->context;
	spin_lock(&instance->lock);
	if ((!urb->status || urb->status == EREMOTEIO)  && urb->actual_length)
    {
/*
      DBG_OUT("Data in ISO URB\n");
*/
      for (i=0;i<ECI_ISO_PACKET_NB;i++)
        {
          if (!urb->iso_frame_desc[i].status && 
              urb->iso_frame_desc[i].actual_length)	{
            if(instance->iso_celbuf_pos)	
              {
                pos = ATM_CELL_SZ - instance->iso_celbuf_pos;
                memcpy(instance->iso_celbuf + instance->iso_celbuf_pos,
                       urb->transfer_buffer + urb->iso_frame_desc[i].offset, pos) ;
                if(!(cell = _uni_cell_fromRaw( ATM_CELL_SZ,
                                               instance->iso_celbuf)))	
                  {
                    ERR_OUT( "not enough memory for new cell\n");
                  } else {
                    if(_uni_cell_list_append(&instance->iso_cells, cell))	
                      {
                        ERR_OUT("Couldn't queue One cell\n");
                        _uni_cell_free(cell);
                      }
                    else
                      received = -1;
                  }
              }
            else
              {
                pos = 0 ;
              }
            for(buf=urb->transfer_buffer + urb->iso_frame_desc[i].offset;
                pos + ATM_CELL_SZ <=urb->iso_frame_desc[i].actual_length;
                pos+=ATM_CELL_SZ) {
              cell = _uni_cell_fromRaw(ATM_CELL_SZ, buf + pos);
              if (!cell) {
                ERR_OUT( "not enough mem for new cell\n") ;
              } else 
                if(_uni_cell_list_append(&instance->iso_cells,cell)) {
                  ERR_OUT("Couldn't queue One cell\n");
                  _uni_cell_free(cell);
                }	 else
                  received = -1;
            }
            if((instance->iso_celbuf_pos = urb->iso_frame_desc[i].actual_length - 
                pos)) {
/*
              DBG_OUT("Saving data for next frame\n");
*/
              memcpy(instance->iso_celbuf, buf +pos,
                     instance->iso_celbuf_pos);
            }
          }
#ifdef DEBUG
          else
            if(urb->iso_frame_desc[i].actual_length)
#endif	/* 	DEBUG	*/
              DBG_OUT("Dropping one frame\tStatus %d\n", 
                      urb->iso_frame_desc[i].status);
        }
    }
	if(received) {
/*
		DBG_OUT("scheduling iso bh from iso callback\n");
*/
		tasklet_schedule (&instance->bh_iso);
	}
					
	end:
	urb->dev = instance->usb_wan_dev;
	urb->pipe = usb_rcvisocpipe(urb->dev, ECI_ISO_PIPE);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	urb->transfer_flags = USB_ISO_ASAP;
#else
	urb->transfer_flags = URB_ISO_ASAP;
#endif
  urb->start_frame       = 0;
	urb->number_of_packets = ECI_ISO_PACKET_NB;
	urb->complete = eci_iso_callback;
	urb->transfer_buffer_length = ECI_ISO_BUFFER_SIZE;
	for(i=0;i < ECI_ISO_PACKET_NB; i ++) {
		urb->iso_frame_desc[i].offset = i * ECI_ISO_PACKET_SIZE;
		urb->iso_frame_desc[i].length = ECI_ISO_PACKET_SIZE;
		urb->iso_frame_desc[i].actual_length = 0 ;
    urb->iso_frame_desc[i].status = 0;
	}
	if(instance->state != ECI_STATE_REMOVING)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		usb_submit_urb(urb);
#else
		usb_submit_urb(urb, GFP_ATOMIC);
#endif
	spin_unlock(&instance->lock);	
}

static int eci_usb_send_urb(struct eci_instance *instance, 
	struct uni_cell_list *cells) {
	int ret;

	if((ret=_uni_cell_list_cat(&instance->bulk_cells, cells))) {
		ERR_OUT("Can't concat cell list, droping cells\n");
		DBG_OUT("Eci_usb_send_urb : ret = %d\n", ret);
		return ret;
	}	else 	{
		tasklet_schedule(&instance->bh_bulk);
		return 0;
	} 
}

static void eci_bh_bulk(unsigned long pinstance) {
	unsigned	char *buf;	/* urb buffer			*/
	int		i;		/* loop counter			*/
	int		buflen;		/* urb buffer len		*/
	int		bufpos;		/* position in urb buffer	*/
	uni_cell_list_crs_t	cell;	/* current computed cell	*/
	struct urb	*urb;		/* urb pointer to sent urb	*/
	struct eci_instance *instance;	/* pointer to instance		*/
	int flags;

/*
  DBG_OUT("eci_bh_bulk: pinstance:0x%x\n",pinstance);
*/
	instance = (struct eci_instance *)pinstance;
	if(instance->state == ECI_STATE_REMOVING) return;
	spin_lock_irqsave(&instance->lock , flags);
	if(instance->bulkisfree)	{
		buflen = ECI_BULK_BUFFER_SIZE;
		urb = instance->bulk_urb;	
		buf = urb->transfer_buffer;
		bufpos = 0;
	 	while(bufpos < (ECI_BULK_BUFFER_SIZE) && 
 			(cell = _uni_cell_list_extract(&instance->bulk_cells))) {
		 		if(cell && cell->raw) {
					memcpy(buf + bufpos, cell->raw, ATM_CELL_SZ);
					for(i=53; i < 64; i++) buf[bufpos + i] = 0xff;
					bufpos += 64;
					_uni_cell_free(cell) ;
				} else {
					if (cell) _uni_cell_free(cell) ;
					break;
				}
		/*if(_uni_cell_list_crs_next(&cell)) break;*/
		}
		usb_fill_bulk_urb(urb, instance->usb_wan_dev, 
			usb_sndbulkpipe(instance->usb_wan_dev, ECI_BULK_PIPE),
			buf, bufpos, eci_bulk_callback, instance);

    DBG_OUT("usb_submit_urb\n");
    DBG_RAW_OUT("buf", buf, bufpos);


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))			
		if(usb_submit_urb(urb))	{
#else
		if(usb_submit_urb(urb, GFP_ATOMIC)) {
#endif		
	/*
		TODO:	Put the cells back in list on error
			need antother way handling cells and
			list 
	*/
				ERR_OUT("Can't submit bulk urb\n");
		} else
			instance->bulkisfree = 0;
	}
	spin_unlock_irqrestore(&instance->lock, flags);
	return;	
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static void eci_bulk_callback(struct urb *urb) {
#else
static void eci_bulk_callback(struct urb *urb, struct pt_regs *regs) {
#endif
	struct eci_instance *instance;

/*
  DBG_OUT("eci_bulk_callback: urb:%p urb->status:%d\n",
          urb,urb->status);
*/

	if (!urb) return ;/*	????	*/
	if(urb->status) {
		ERR_OUT("Error on Bulk URB, status %d\n", urb->status);
	}
	instance = (struct eci_instance *)urb->context;
	if(instance->state == ECI_STATE_REMOVING) return;
	spin_lock(&instance->lock);
	instance->bulkisfree = 1;
	if(_uni_cell_list_nbcells(&instance->bulk_cells)) {
		tasklet_schedule(&instance->bh_bulk);
	} else {
		/*	Schedule ATM in case of pending data
			as many schedule may have been canceled
			by one call to bh_atm	*/
/*
		DBG_OUT("Rescheduling ATM BH from bulk callback");
*/
		tasklet_schedule(&instance->bh_atm);
	}
	spin_unlock(&instance->lock);
}
/**********************************************************************
		END USB CODE
*************************************************************************/

/*----------------------------------------------------------------------
 *
 * Complete the AAL5 frame received from ATM layer
 * and split it into several UNI cells
 * We need to format the AAL5 trailer
 * Then enqueue each cells
 *	vpi 	IN: VPI,  vci    IN: VCI
 */
static int _eci_tx_aal5(int  vpi, int vci, struct eci_instance *	pinstance,
		struct sk_buff *	pskb) {
	int		lv_rc 		= 0 ;
	int		lv_nbsent ;
	uni_cell_list_t 	lv_list ;
	
	lv_rc = _uni_cell_list_init(&lv_list) ;
	if (lv_rc) return lv_rc ;

	/* Split AAL5 frame */
	lv_rc = _aal5_to_cells(vpi, vci, pskb->len, pskb->data, &lv_list) ;
	if (lv_rc) {
		ERR_OUT("Failed to split aal5 frame\n") ;
		_uni_cell_list_reset(&lv_list) ;
		return lv_rc ;
	}

	/* Send the Cells to USB layer */
	lv_nbsent = eci_usb_send_urb(pinstance, &lv_list) ;

	if(lv_rc = _uni_cell_list_nbcells(&lv_list)) {
	       ERR_OUT( "Not all the cells where sent (%d/%d)\n", lv_nbsent,
			       _uni_cell_list_nbcells(&lv_list)) ;
		_uni_cell_list_reset(&lv_list) ;
	}

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
	return lv_rc ;
}

/*----------------------------------------------------------------------
 * 
 * Send Cell to ATM
 * this need to be fully rewritten !!
 */
static int eci_atm_receive_cell(
		struct eci_instance * pinstance,
		uni_cell_list_t *		plist
) {
	uni_cell_t *		lp_cell		= NULL ;
 	aal5_t *		lp_aal5 	= NULL ;
	int			lv_rc ;
	struct atm_vcc *vcc;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22))
	struct sock *s;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	struct hlist_head *head;
	struct hlist_node *node;
	int i;
#endif
#endif

	/* Check Interface */
	if (!pinstance || !plist)
		return -EINVAL ;

	/* Check if VCC available */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23))
	if (!(vcc = pinstance->atm_dev->last)){
		ERR_OUT("No opened VC\n") ;
		return -ENXIO ;
	}
#else
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	read_lock(&vcc_sklist_lock);
	for(s = vcc_sklist; s; s = s->next)
	{
		vcc = s->protinfo.af_atm;
		if(vcc->dev->dev_data == pinstance) break;
	}
	read_unlock(&vcc_sklist_lock);
	DBG_OUT("s = %x\n",s);
	if(!s)	return -ENXIO ;
	DBG_OUT("Found VCC %d %d\n", vcc->vci, vcc->vpi);
#else
	for(i=0; i< VCC_HTABLE_SIZE; i++) {
		head = &vcc_hash[i];
		sk_for_each(s, node, head) {
				vcc = atm_sk(s);
				if(vcc->dev->dev_data == pinstance) break;
		}
		if(vcc->dev->dev_data == pinstance) break;
	}
	if(vcc->dev->dev_data != pinstance) return -ENXIO;
#endif
#endif
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

	/* Manage a complete AAL5 */
		do {
			lp_cell = _uni_cell_list_extract(plist) ;
			if (!lp_cell) {
/*
				DBG_OUT("No more cell\n") ;
*/
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
		_eci_rx_aal5(pinstance, lp_aal5, vcc) ;

		/* Reset AAL5 */
		_aal5_clean(lp_aal5) ;

end:
 	/* Manage AAL5 backlog */
 	if (_aal5_getNbCell(lp_aal5) && !_aal5_iscomplete(lp_aal5)) {
 		pinstance->pbklogaal5 = lp_aal5 ;
/*
 		DBG_OUT(
 			"store bklog AAL5 with [%d] cells\n",
 			_aal5_getNbCell(lp_aal5)) ;
*/
 	} else {
 		_aal5_free(lp_aal5) ;
 	}
	if(_uni_cell_list_nbcells(plist))
	{
		tasklet_schedule(&pinstance->bh_iso);
	}
	//_uni_cell_list_reset(plist);
	return 0 ;
}

/*----------------------------------------------------------------------
 *
 * Send data to ATM
 *
 * Gotta add some check for which vcc somewhere !!!
 *
 */
static int _eci_rx_aal5(struct eci_instance *	pinstance,
		aal5_t *		paal5,struct atm_vcc *vcc) {
	uni_cell_t *		lp_cell		= NULL ;
	int			lv_vpi ;
	int			lv_vci ;
	size_t			lv_size ;
	struct sk_buff *	lp_skb		= NULL ;
	struct sock *s;
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
	lp_skb = atm_alloc_charge(vcc, lv_size, GFP_ATOMIC);
	if (!lp_skb) {
		ERR_OUT("not enought mem for new skb\n") ;
		return -ENOMEM ;
	}

	/* Init SKB */
	skb_put(lp_skb, lv_size) ;
	ATM_SKB(lp_skb)->vcc = vcc ;
	do_gettimeofday(&lp_skb->stamp);
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
	vcc->push(pinstance->pcurvcc, lp_skb) ;
	atomic_inc(&vcc->stats->rx) ;
	return 0 ;
}

/*
 * Toolz for CRC computing (from Benoit Papillault)
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
static inline uni_cell_t * _uni_cell_alloc(void) {
	
	uni_cell_t *lp_cell = NULL ;
	
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
		ERR_OUT("No availlable cell create new one\n") ;
		lp_cell = (uni_cell_t *) kmalloc(
				sizeof(uni_cell_t),
				GFP_ATOMIC) ;
		if (!lp_cell) {
			ERR_OUT("Not enought memory for new cell\n") ;
			return NULL ;
		}
	}

	return lp_cell ;
}

/*----------------------------------------------------------------------*/

/*
 * Built a new UNI cell from raw data
 * Warning : buffer size MUST be ATM_CELL_SZ bytes
 */
static inline uni_cell_t * _uni_cell_fromRaw(
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
static inline void _uni_cell_free(
		uni_cell_t * pcell
) {
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
}

/*----------------------------------------------------------------------*/

/* Format a cell */
static inline int _uni_cell_format(
	int			vpi,	/* IN: VPI			*/
	int			vci,	/* IN: VCI			*/
	bool			islast,	/* IN: Flag for last cell	*/
	size_t			size,	/* IN: Payload size		*/
	byte *	 		pdata,	/* IN: Payload data		*/
	uni_cell_t *		pcell	/* OUT: Formated UNI cell	*/
) {
	int		lv_padd	= 0 ;
	byte *		lp_cell	= NULL ;

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

	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Get VPI from cell */
static inline int _uni_cell_getVpi(
		uni_cell_t *		pcell	/* IN: the cell		*/
) {
	byte *	lp_raw = NULL ;
	int	lv_vpi ;

	/* Check Interface */
	if (!pcell) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	lp_raw = pcell->raw ;

	lv_vpi = ((0x0f & lp_raw[0]) << 4) | (lp_raw[1] >> 4) ;

	return lv_vpi ;
}

/*----------------------------------------------------------------------*/

/* Get VCI from cell */
static inline int _uni_cell_getVci(
		uni_cell_t *		pcell	/* IN: the cell		*/
) {
	byte *	lp_raw = NULL ;
	int	lv_vci ;

	/* Check Interface */
	if (!pcell) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}

	lp_raw = pcell->raw ;
	
	lv_vci = 
		((((0x0f & lp_raw[1]) << 4) | (lp_raw[2] >> 4)) << 8) |
	       	( ((0x0f & lp_raw[2]) << 4) | (lp_raw[3] >> 4)) ;

	return lv_vci ;
}

/*----------------------------------------------------------------------*/

/* check if this is the last cell */
static inline bool _uni_cell_islast(
		uni_cell_t *		pcell	/* IN: the cell		*/
) {
	bool lv_res ;

	/* check interface */
	if (!pcell) {
		ERR_OUT("interface error\n") ;
		return true ;
	}

	lv_res = ((pcell->raw[3] & 0x02) == 0x02) ;

	return lv_res ;
}

/*----------------------------------------------------------------------*/

/* Get The Payload */
static inline byte * _uni_cell_getPayload(
		uni_cell_t *		pcell	/* in: the cell		*/
) {

	/* check interface */
	if (!pcell) {
		ERR_OUT("Interface Error\n") ;
		return NULL ;
	}

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
static inline void _uni_cell_setHEC(
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
static inline bool _uni_cell_chkHEC(
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
static inline int _uni_cell_qinit(uni_cell_q_t *pcellq) {

	/* Check Interface */
	if (!pcellq) {
		ERR_OUT("Interface Error\n") ;
		return -EINVAL ;
	}
	
	pcellq->nbcells		= 0 ;
	pcellq->first		= NULL ;
	pcellq->last		= NULL ;

	return 0 ;
}
/*----------------------------------------------------------------------*/

/* Alloc a new queue */
static inline uni_cell_q_t * _uni_cell_qalloc(void) {
	
	uni_cell_q_t * lp_cellq = NULL ;

	lp_cellq = (uni_cell_q_t *) kmalloc(
			sizeof(uni_cell_q_t),
			GFP_ATOMIC) ;
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
	
	return lp_cellq ;
}

/*----------------------------------------------------------------------*/

/* 
 * Free a queue :
 * 	free also each remaining cells
 */
static inline void _uni_cell_qfree(uni_cell_q_t *pcellq) {

	uni_cell_t *	lp_cell 	= NULL ;
	uni_cell_t *	lp_cellnxt 	= NULL ;
	

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

}

/*----------------------------------------------------------------------*/

/*
 * Check if the queue is empty
 */
static inline bool _uni_cell_qisempty(
		uni_cell_q_t * pcellq
) {
	return (pcellq->nbcells == 0) ;
}

/*----------------------------------------------------------------------*/

/*
 * Push new UNI Cell in Queue
 */
static inline int _uni_cell_qpush(
		uni_cell_t *	pcell,	/* IN: Cell to insert	*/
		uni_cell_q_t *	pcellq	/* IN: Target queue	*/
) {

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

	return 0 ;
}

/*----------------------------------------------------------------------*/

/*
 * Pop an UNI Cell from Queue :
 * 	return NULL when empty or invalid queue
 */
static inline uni_cell_t * _uni_cell_qpop(
		uni_cell_q_t *	pcellq	/* IN: Target queue	*/
) {
	uni_cell_t * lp_cell = NULL ;
	

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
	
	return lp_cell ;
}

/*----------------------------------------------------------------------*/

/*
 * Get the UNI Cell on head Queue :
 * 	return NULL when empty or invalid queue
 */
static inline uni_cell_t * _uni_cell_qhead(
		uni_cell_q_t *	pcellq	/* IN: Target queue	*/
) {
	uni_cell_t * lp_cell = NULL ;
	

	/* Check Interface */
	if (!pcellq) {
		ERR_OUT("Interface Error\n") ;
		return NULL ;
	}

	lp_cell = pcellq->first ;

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
static inline int _uni_cell_list_init(uni_cell_list_t *plist) {
	if (!plist) return -EINVAL ;
	memset(plist, 0x00, sizeof(uni_cell_list_t)) ;
	return 0 ;
}

/*----------------------------------------------------------------------*/

/* Reset a list (free each cells) */

static inline void _uni_cell_list_reset(uni_cell_list_t *plist) {
	uni_cell_t *	lp_cell = NULL ;

/*	if (!plist) return ;*/

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

static inline uni_cell_list_t * _uni_cell_list_alloc(void) {
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

static inline void _uni_cell_list_free(uni_cell_list_t *plist) {

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
static inline uni_cell_t * _uni_cell_list_extract(uni_cell_list_t *plist) {
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

static inline int _uni_cell_list_insert(uni_cell_list_t *plist, uni_cell_t *pcell) {
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

static inline int _uni_cell_list_append(uni_cell_list_t *plist, uni_cell_t *pcell) {
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
static inline int _uni_cell_list_cat(
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
static inline int _aal5_init(aal5_t * paal5) {
	
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
static inline aal5_t * _aal5_alloc(void) {
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
static inline void _aal5_free(aal5_t *paal5) {
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
static inline void _aal5_clean(aal5_t *paal5) {
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
static inline int _aal5_add_cell(
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
static inline uni_cell_t * _aal5_get_next(aal5_t *paal5) {
	
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

