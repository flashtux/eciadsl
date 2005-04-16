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
*/

#ifndef GSINTERFACE_H
#define GSINTERFACE_H

/* Vendor/ProdID for "ECI Telecom USB ADSL Loader" */
#define EZUSB_NAME    "EZUSB USB ADSL Loader"
#define EZUSB_VENDOR  0x0547 /* 0x0547 */
#define EZUSB_PRODUCT 0x2131 /* 0x2131 */

/* Vendor/ProdID for "ECI Telecom USB ADSL WAN Modem" */
#define GS_NAME       "GlobeSpan GS7070 USB ADSL WAN Modem"
#define GS_VENDOR     0x0915 /* 0x0915 */
#define GS_PRODUCT    0x8000 /* 0x8000 */

#define CONTROLCODEBUFFERSIZE 20
/* Piucked at random, I never get more that three controlcodes in a
   row so 20, if there are 20 then somethings gone badly wrong!
*/

/* Error Codes */
#define GSLINEDROP -1
/* line dropped (e.g. you unplugged the modem from the phone line) */
#define GSSYNCERROR -2
/* can happed after a lkine drop? */

/* eci_device_chipset */
typedef enum device_chiset{
       ECIDC_GS7070  = 0,  /* GS7070 chipset - standard one */
       ECIDC_GS7470  = 1  /* GS7470 chipset - new modems */
} eci_device_chiset;

struct eci_device_t
{
	eci_device_chiset eci_modem_chipset;
	/* ep numbers */
	unsigned char eci_int_ep;
	unsigned char eci_iso_ep;
	unsigned char eci_in2_ep;
	unsigned char eci_bulk_ep;	
	/* interfaces # */
	unsigned short int alt_interface_synch;
	unsigned short int alt_interface_ppp;	
	/* max size of data endpoint (Windows driver uses 448). */
	short int eci_iso_packet_size;
	/* number of ISO packet per URB (Windows driver uses 10) */
	short int eci_nb_iso_packet;
	/* number of simultaneous URB submitted to data endpoint (Windows drv : 20) */
	short int eci_nb_pkt_ep_data_in; 	
	/* EP INT output buffer size */
	short int ep_int_out_buf_size;
	/* EP INT data start point */
	short int ep_int_data_start_point;
	/* BULK RESPONSE VALUE */
	unsigned short bulk_response_value;	
	/* AAL5 INPUT CELL STRUCTURE */
	short int cell_r_size;
	short int cell_r_hdr;
	short int cell_r_data;
	/* AAL5 OUTPUT CELL STRUCTURE */
	short int cell_w_size;
	short int cell_w_hdr;
	short int cell_w_data;
	short int pad_size;
};

/* structure for aal5 header decoding - kolja*/
struct aal5_header_st {
		unsigned int vpi;
		unsigned int vci;
		unsigned int pti;
};

int  gsSetControl(unsigned char* buffer);
void gsGetResponse(unsigned char* buffer);
void allocateGSint(void);
void deallocateGSint(void);
void getAal5HeaderStructure(const unsigned char* aal5Header, struct aal5_header_st* aal5HeaderOut);


/* set eci modem_chipset - kolja */
void set_eci_modem_chipset(char* chipset);

const char * get_chipset_descr(eci_device_chiset eci_chipset);
#endif
