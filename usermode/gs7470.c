/*
  Author : kolja <gava@bergamoblog.it>
  Creation : 29/11/2003
  Licence : GPL

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************
  GlobeSpan GS7470 Chipset Interface
     cantain all the information related to device 
*/

#include <stdlib.h>
#include <stdio.h>
#include "gsinterface.h"
#include "gs7470.h"

extern struct eci_device_t eci_device;

/* GSControlINT variable */
GS7470ControlINT* gs7470;
/* GSControlReg pointer list for connections type (see CUR_CONN_#### defines) for code 0x7311*/
GS7470ControlReg* connCtrlReg73[4];
/* GSControlReg pointer list for connections type (see CUR_CONN_#### defines) for code 0x5311*/
GS7470ControlReg* connCtrlReg53[4];

/* Initialize Device Parameters */
void gs7470InitParams(void){
	
	/* Vendor/ProdID for "ECI Telecom USB ADSL WAN Modem" */
	eci_device.eci_modem_chipset = ECIDC_GS7470;
	/* endpoint numbers */
	eci_device.eci_int_ep = 0x81;
	eci_device.eci_in_ep = 0x82;
	eci_device.eci_out_ep = 0x04;	
	eci_device.use_datain_iso_urb = 1;

	/* interfaces # */
	eci_device.alt_interface_synch = 5;
	eci_device.alt_interface_ppp = 5;

	/* PPP DEFINES */
	/* max size of data endpoint (Windows driver uses 448). */
	eci_device.eci_iso_packet_size = 240;
	/* number of ISO packet per URB (Windows driver uses 10) */
	eci_device.eci_nb_iso_packet = 10;
	/* number of simultaneous URB submitted to data endpoint (Windows drv : 20) */
	eci_device.eci_nb_pkt_ep_data_in = 20; 	
	/* EP INT output buffer size */
	eci_device.ep_int_out_buf_size = 40;
	/* EP INT data start point */
	eci_device.ep_int_data_start_point = 3;
	/* EP INT data size */
	eci_device.ep_int_data_size=32;	
	
	/* BULK RESPONSE VALUE */
	eci_device.bulk_response_value = 0x0c02;

	/* AAL5 INPUT CELL STRUCTURE */
	eci_device.cell_r_size = 51;
	eci_device.cell_r_hdr  = 03;
	eci_device.cell_r_data = 48;

	/* AAL5 OUTPUT CELL STRUCTURE */
	eci_device.cell_w_size = 53;
	eci_device.cell_w_hdr  = 05;
	eci_device.cell_w_data = 48;

	eci_device.pad_size = 0;
}

/* get structure for aal5 header */
static inline void getAal5HeaderStructure7470(const unsigned char* aal5Header,
                                struct aal5_header_st* aal5HeaderOut)
{
	aal5HeaderOut->vpi = ((aal5Header[0]  >>  4) & 0x0f);
	aal5HeaderOut->vci = (((aal5Header[1]  <<  4) & 0xf0) | ((aal5Header[2] >> 4) & 0x0f));
	aal5HeaderOut->pti = ( aal5Header[2] & 0x0f);
}
