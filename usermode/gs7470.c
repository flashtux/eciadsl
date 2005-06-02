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
	eci_device.eci_iso_ep = 0x82;
	eci_device.eci_in2_ep = 0x83;	
	eci_device.eci_bulk_ep = 0x04;

	/* interfaces # */
	eci_device.alt_interface_synch = 1;
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

/* Allocate a regster and setup some values */
GS7470ControlReg* allocateGS7470CtrlReg(char cdType, unsigned int connType){	
	GS7470ControlReg* rp=(GS7470ControlReg*)malloc(sizeof(GS7470ControlReg));
	switch(connType){
		case CUR_CONN_F343 :
			switch(cdType){
				case 0x73 :
					rp->replace = malloc(2);
					rp->replace[0] = 0x63;
					rp->replace[1] = 0x01;
					rp->maxRepPosition=0;
					break;
				case 0x53 :
					rp->replace = malloc(2);
					rp->replace[0] = 0x43;
					rp->replace[1] = 0x01;
					rp->maxRepPosition=0;
					break;
				default: ;
			}
			break;
		case CUR_CONN_7341 :
			switch(cdType){
				case 0x73 :
					rp->replace = malloc(6);
					rp->replace[0] = 0x63;
					rp->replace[1] = 0x01;
					rp->replace[2] = 0xe3;
					rp->replace[3] = 0x47;
					rp->replace[4] = 0x63;
					rp->replace[5] = 0x51;
					rp->maxRepPosition=2;
					break;
				case 0x53 :
					rp->replace = malloc(8);
					rp->replace[0] = 0xc3;
					rp->replace[1] = 0xb5;
					rp->replace[2] = 0xc3;
					rp->replace[3] = 0x53;
					rp->replace[4] = 0x43;
					rp->replace[5] = 0x4f;
					rp->replace[6] = 0xc3;
					rp->replace[7] = 0x39;
					rp->maxRepPosition=3;					
					break;
				default: ;
			}
			break;
		case CUR_CONN_734D :
			switch(cdType){
				case 0x73 :
					rp->replace = malloc(2);
					rp->replace[0] = 0xe3;
					rp->replace[1] = 0x19;
					rp->maxRepPosition=0;
					break;
				case 0x53 :
					rp->replace = 0;
					rp->maxRepPosition=-1;
					break;
				default: ;
			}
			break;
		case CUR_CONN_F34F :
			switch(cdType){
				case 0x73 :
					rp->replace = malloc(2);
					rp->replace[0] = 0xe3; /* o 6347 o e345 */
					rp->replace[1] = 0x47;
					rp->maxRepPosition=0;
					break;
				case 0x53 :
					rp->replace = 0;
					rp->maxRepPosition=-1;
					break;
				default: ;
			}
			break;
		default : ;		
	}
	rp->count=0;
	rp->curRepPosition=0;
	return(rp);
}

/* Clean up a register */
void deallocateGS7470CtrlRegs(GS7470ControlReg* gscrl[]){
	unsigned int i;
	for (i=0; i< sizeof(gscrl); i++){
		free(gscrl[i]);
	}
}

/* Allocate the gs7470 interrupt handler and do some initilisation */
void allocateGS7470int(void)
{
	/*which way round should a malloc be?*/
	gs7470=(GS7470ControlINT*)malloc(sizeof(GS7470ControlINT));

	/* Allocate GSCtrlReg for cod 0x7311 */
	connCtrlReg73[CUR_CONN_F343] = allocateGS7470CtrlReg(0x73, CUR_CONN_F343);
	connCtrlReg73[CUR_CONN_7341] = allocateGS7470CtrlReg(0x73, CUR_CONN_7341);	
	connCtrlReg73[CUR_CONN_734D] = allocateGS7470CtrlReg(0x73, CUR_CONN_734D);	
	connCtrlReg73[CUR_CONN_F34F] = allocateGS7470CtrlReg(0x73, CUR_CONN_F34F);

	/* Allocate GSCtrlReg for cod 0x5311 */
	connCtrlReg53[CUR_CONN_F343] = allocateGS7470CtrlReg(0x53, CUR_CONN_F343);
	connCtrlReg53[CUR_CONN_7341] = allocateGS7470CtrlReg(0x53, CUR_CONN_7341);	
	connCtrlReg53[CUR_CONN_734D] = allocateGS7470CtrlReg(0x53, CUR_CONN_734D);	
	connCtrlReg53[CUR_CONN_F34F] = allocateGS7470CtrlReg(0x53, CUR_CONN_F34F);

	gs7470->controlcodecount=-1;
	gs7470->controlseqcount=0;
	gs7470->curReplace73=0;
	gs7470->curReplace53=0;
	gs7470->currentConnection = CUR_CONN_UNKNOWN;
}

/* Deallocate the gs7470 interrupt handler and clean up */
void deallocateGS7470int(){
	gs7470->curReplace73=0;
	gs7470->curReplace53=0;
	free(gs7470);
	deallocateGS7470CtrlRegs(connCtrlReg73);
	deallocateGS7470CtrlRegs(connCtrlReg53);
}

/* Manages commands sent from the modem */

int gs7470SetControl(unsigned char* inBuffer){
	unsigned short controlcode=(inBuffer[0]<<8 )|inBuffer[1];
	/*  Check for junk/unknown stuff, because it probably means that the line
	    has dropped */
	if (controlcode==0x0C0C)
		return(0);
	/* check to see if we have a new control code to put into the buffer */
	switch(controlcode){
		case 0xf343 :
			gs7470->currentConnection = CUR_CONN_F343; 
			gs7470->curReplace73 = connCtrlReg73[CUR_CONN_F343];
			gs7470->curReplace73->count = 0;
			gs7470->curReplace73->curRepPosition = 0;
			gs7470->curReplace53 = connCtrlReg53[CUR_CONN_F343];
			gs7470->curReplace53->count = 0;
			gs7470->curReplace53->curRepPosition = 0;
			break;
		case 0x7341 :
			gs7470->currentConnection = CUR_CONN_7341;
			gs7470->curReplace73 = connCtrlReg73[CUR_CONN_7341];
			gs7470->curReplace73->count = 0;
			gs7470->curReplace73->curRepPosition = 0;
			gs7470->curReplace53 = connCtrlReg53[CUR_CONN_7341];
			gs7470->curReplace53->count = 0;
			gs7470->curReplace53->curRepPosition = 0;
			break;
		case 0x734d :
			gs7470->currentConnection = CUR_CONN_734D;
			gs7470->curReplace73 = connCtrlReg73[CUR_CONN_734D];
			gs7470->curReplace73->count = 0;
			gs7470->curReplace73->curRepPosition = 0;
			gs7470->curReplace53 = connCtrlReg53[CUR_CONN_734D];
			gs7470->curReplace53->count = 0;
			gs7470->curReplace53->curRepPosition = 0;
			break;
		case 0xf34f :
			gs7470->currentConnection = CUR_CONN_F34F;
			gs7470->curReplace73 = connCtrlReg73[CUR_CONN_F34F];
			gs7470->curReplace73->count = 0;
			gs7470->curReplace73->curRepPosition = 0;
			gs7470->curReplace53 = connCtrlReg53[CUR_CONN_F34F];
			gs7470->curReplace53->count = 0;
			gs7470->curReplace53->curRepPosition = 0;
			break;
		default: ;
	}
	return(0);
}

/* Returns the correct response to a control command sent by the modem,
   gsSetControl must be called first so that we are in the correct
   mode for the response.
*/
void gs7470GetResponse(unsigned char* buffer)
{
	if ((buffer[0]!=0x73 && buffer[0]!=0x53) || buffer[1]!=0x11)
		return; /* nothing to do */
	switch (buffer[0])
	{
		case 0x53:
			if (gs7470->curReplace53!=0){
				if (gs7470->curReplace53->count==0 && gs7470->curReplace73->maxRepPosition!=-1 && gs7470->curReplace73->count!=0){
					if (gs7470->curReplace73->curRepPosition < gs7470->curReplace73->maxRepPosition){
						gs7470->curReplace73->curRepPosition++;
					}else{
						gs7470->curReplace73->curRepPosition=0;
					}
					gs7470->curReplace73->count = 0;
				}
				/*BUG : check if curReplace53 has some replace codes */
				if (gs7470->curReplace53->maxRepPosition >0){
					gs7470->curReplace53->count++;
					if (gs7470->curReplace53->count<=2){
						/*don't need to replace anything the first two times around*/
						break;
					}
					buffer[0]= gs7470->curReplace53->replace[(gs7470->curReplace53->curRepPosition *2)];
					buffer[1]= gs7470->curReplace53->replace[(gs7470->curReplace53->curRepPosition *2)+1];
				}
			}
			break;
		case 0x73:
			if (gs7470->curReplace73!=0){
				if (gs7470->curReplace73->count==0 && gs7470->curReplace53->maxRepPosition!=-1 && gs7470->curReplace53->count!=0){
					if (gs7470->curReplace53->curRepPosition < gs7470->curReplace53->maxRepPosition){
						gs7470->curReplace53->curRepPosition++;
					}else{
						gs7470->curReplace53->curRepPosition=0;
					}
					gs7470->curReplace53->count = 0;
				}
				gs7470->curReplace73->count++;
				if (gs7470->curReplace73->count<=2){
					/*don't need to replace anything the first two times around*/
					break;
				}
				buffer[0]= gs7470->curReplace73->replace[(gs7470->curReplace73->curRepPosition *2)];
				buffer[1]= gs7470->curReplace73->replace[(gs7470->curReplace73->curRepPosition *2)+1];
			}
			break;
		default:;
	}
}


/* get structure for aal5 header */
void getAal5HeaderStructure7470(const unsigned char* aal5Header,
                                struct aal5_header_st* aal5HeaderOut)
{
	aal5HeaderOut->vpi = ((aal5Header[0]  >>  4) & 0x0f);
	aal5HeaderOut->vci = (((aal5Header[1]  <<  4) & 0xf0) | ((aal5Header[2] >> 4) & 0x0f));
	aal5HeaderOut->pti = ( aal5Header[2] & 0x0f);
}
