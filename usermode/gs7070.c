/*
  Author : kolja <gava@bergamoblog.it>
  Creation : 29/11/2003
  Licence : GPL

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************
  GlobeSpan GS7070 Chipset Interface
     cantain all the information related to device 

  Changes:
  Oliverthered: oliverthered:oliverthered.com

  Corrected a few typos and added some aditional comments about the expected
  code sequences and there responses. This version now responds to the modem
  exactly the same as the Windows driver Annax A, G.DMT vci=0 vpi=38.
*/

#include <stdlib.h>
#include <stdio.h>
#include "gsinterface.h"
#include "gs7070.h"

extern struct eci_device_t eci_device;

/* GSControlINT variable */
GS7070ControlINT* gs7070;

/* Initialize Device Parameters */
void gs7070InitParams(void){
	
	/* Vendor/ProdID for "ECI Telecom USB ADSL WAN Modem" */
	eci_device.eci_modem_chipset = ECIDC_GS7070;

	/* endpoint numbers */
	eci_device.eci_int_ep = 0x86;
	eci_device.eci_iso_ep = 0x88;
	eci_device.eci_in2_ep = 0x89;
	eci_device.eci_bulk_ep = 0x02;

	/* interfaces # */
	eci_device.alt_interface_synch = 4;
	eci_device.alt_interface_ppp = 4;

	/* PPP DEFINES */
	/* max size of data endpoint (Windows driver uses 448). */
	eci_device.eci_iso_packet_size = 448;
	/* number of ISO packet per URB (Windows driver uses 10) */
	eci_device.eci_nb_iso_packet = 10;
	/* number of simultaneous URB submitted to data endpoint (Windows drv : 20) */
	eci_device.eci_nb_pkt_ep_data_in = 20; 		
	/* EP INT output buffer size */
	eci_device.ep_int_out_buf_size = 40;
	/* EP INT data start point */
	eci_device.ep_int_data_start_point = 5;
	/* EP INT data size */
	eci_device.ep_int_data_size=30;	

	/* BULK RESPONSE VALUE */
	eci_device.bulk_response_value = 0x0c02;

	/* AAL5 INPUT CELL STRUCTURE */
	eci_device.cell_r_size = 53;
	eci_device.cell_r_hdr  = 05;
	eci_device.cell_r_data = 48;

	/* AAL5 OUTPUT CELL STRUCTURE */
	eci_device.cell_w_size = 53;
	eci_device.cell_w_hdr  = 05;
	eci_device.cell_w_data = 48;

	eci_device.pad_size = 11;
}

/* Manages commands sent from the modem */
int gs7070SetControl(unsigned char* buffer)
{
	unsigned short controlcode=(buffer[0]<<8 )|buffer[1];
	/*  Check for junk/unknown stuff, because it probably means that the line
	    has dropped */
	if (controlcode==0x0C0C)
		return(0);
	if (controlcode==0x7311 || controlcode==0x5311)
	{
		if (controlcode==0x7311 && gs7070->replace73->count==5)
		{
			gs7070->replace73->count=0;
			if (gs7070->controlcodecount==1 && gs7070->controlcodes[1]==0x7341)
			{
				gs7070->count7341++;
				if (gs7070->count7341==3)
				{
					gs7070->replace73->replace[0]=0x73;
					gs7070->replace73->replace[1]=0x0F;
				}
			}
		}
		else
			if (controlcode==0x5311 && gs7070->replace53->count==5)
			{
				gs7070->replace53->count=0;
				if (gs7070->controlcodecount==1 && gs7070->controlcodes[1]==0x7341)
				{
					if (gs7070->replace53->replace[0]==0xC3
						&& gs7070->replace53->replace[1]==0x39)
					{
						gs7070->replace53->replace[0]=0x43;
						gs7070->replace53->replace[1]=0x01;
					}
				}

			}
		return(0); /* nothing to do */
	}

	/* check to see if we have a new control code to put into the buffer */
	if (gs7070->controlcodecount==-1)
	{
		gs7070->controlcodecount++;
		gs7070->controlcodes[gs7070->controlcodecount]=controlcode;
		gs7070->controlseqcount=1;
	}
	else
	{
		if (gs7070->controlcodes[gs7070->controlcodecount]!=controlcode)
		{
			if (gs7070->controlcodecount>=CONTROLCODEBUFFERSIZE-1)
			{
				printf("Too many control codes without a reset, expect a fatal error!\n");
				return(GSSYNCERROR);
			}

			if (gs7070->controlseqcount!=3)
				printf("Unexpected control code change %d\n",
						gs7070->controlseqcount);
			/* See if the we have received a reset command */
			/* also a 0xF343? */
			if (controlcode==0xF301)
			{ /* F301 is some kind of reset command */
				printf("Command change received\n");
				gs7070->controlcodecount=0;
/*				gs7070->controlseqcount=0; */
			}
			else
			/* No reset so add the new command onto the end of the control
			   code list */
				gs7070->controlcodecount++;
			gs7070->controlcodes[gs7070->controlcodecount]=controlcode;
			gs7070->controlseqcount=1;
		}
		else
			gs7070->controlseqcount++;
	}
/* Big nasty state catcher comming up!!
   One day, maybe sometime in the future this can be replaced with some
   propper cntrol code!!!
*/
	if (gs7070->controlseqcount==1)
	{
		/*
		 I think the first down the line is a f343? hmm...
		 I get the odd F343 code that needs handeling
		 0xF343 looks like a 'opps I didn't expet that, start again code',
		 I get lots of them if I don't generate the correct responses
		 I should imagine the correct response is 63 01 and 43 01
		 setup the expected responses for the controlregs;
		 the list of known control codes are as follows

		 also sometimes if the control sequence is mucked up I get a single
		 0xF301 always 0xF301 (reset) followed by
		 0xf3 13
		 0xf3 4f --0x7311 should responed 0x63 53(possibly the first time)
		   or 0xE3 0x51
		 or
		 0x73 4D  -- 0x7311 should responed 0x63 0B(possibley the first time)
		   or 0xE3 0B
		 or
		 0x73 41 --This is usually the first control seq I get after the modem
		 has connected to the ISP

		 When the line drops the modem sends junk....

		 And I should respond with ????? something to drop the connection?
		 response to 0x73 41 is usually
			73 11 (63 01)
			53 11 (c3 39)
			73 11 (63 01)
			53 11 (43 01)
			73 11 (63 01)
			53 11 (43 01)
			73 11 (73 0f)
		*/
		if (gs7070->controlcodecount==0)
		{
			gs7070->replace53->replace[0]=0x43;
			gs7070->replace53->replace[1]=0x01;
			gs7070->replace53->count=0;
			gs7070->replace73->replace[0]=0x63;
			gs7070->replace73->replace[1]=0x01;
			gs7070->replace73->count=0;
		}
		else
			if (gs7070->controlcodecount==1)
			{
				switch (gs7070->controlcodes[0])
				{
					case 0xf301:
						printf("0xF301\n");
						switch (gs7070->controlcodes[1])
						{
							case 0x734D:
								printf("0x734D\n");
								/* this is sometime 0x63 53 (only ever the first
								   time for me and only sometimes)*/
								if (gs7070->count734D==0){
									gs7070->replace73->replace[0]=0x63;
									gs7070->replace73->replace[1]=0x0B;

									gs7070->count734D=1;
								}
								else
								{
									gs7070->replace73->replace[0]=0xE3;
									gs7070->replace73->replace[1]=0x0B;
								}
								gs7070->replace73->count=0;
								break;
							case 0x7341:
								printf("0x7341\n");
								gs7070->count7341=0;
								gs7070->replace73->replace[0]=0x63;
								gs7070->replace73->replace[1]=0x01;
								gs7070->replace73->count=0;
								gs7070->replace53->replace[0]=0xc3;
								gs7070->replace53->replace[1]=0x39;
								gs7070->replace53->count=0;
								break;
							case 0xF313:
								printf("0xF313\n");
								/* This isn't an end code and should be followed
								   by F3 4f so don't report */
								break;
							default:
								;
								/* default: (ANSI C fix) */
								/* ahhhh I don't know what the modem is trying
								   to tell me! */
						};
						break;
					default:
						printf("Unknown control code sequence count1 %04x\n",
								gs7070->controlcodes[1]);
				};
			}
			else
				if (gs7070->controlcodecount==2)
				{
					/* need to add the rest of the codes in here!! */
					switch (gs7070->controlcodes[2])
					{
						case 0xF34F:
							printf("0xF34F\n");
							gs7070->replace73->replace[0]=0xE3;
							gs7070->replace73->replace[1]=0x51;
							gs7070->replace73->count=0;
							break;
						default:
							printf("More control codes than expected count%d %04x \n",
									gs7070->controlcodecount,
									gs7070->controlcodes[gs7070->controlcodecount]);
					};
				}
				else
					printf("Unknown control code sequence count0 %04x\n",
							gs7070->controlcodes[0]);

	}
	else
		if (gs7070->controlseqcount==4)
			printf("Expected change of control code %04x\n",
					gs7070->controlcodes[3]);
	return(0);
}

void replaceme(GS7070ControlReg* rp, unsigned char* data)
{
	rp->count++;
	if (rp->count<=2)
		return; /*don't need to replace anything the first two times around*/
	data[0]=rp->replace[0];
	data[1]=rp->replace[1];
}

/* Returns the correct response to a control command sent by the modem,
   gs7070SetControl must be called first so that we are in the correct
   mode for the response.
*/
void gs7070GetResponse(unsigned char* buffer)
{
	if ((buffer[0]!=0x73 && buffer[0]!=0x53) || buffer[1]!=0x11)
		return; /* nothing to do */
	switch (buffer[0])
	{
		case 0x53:
			replaceme(gs7070->replace53, buffer);
			break;
		case 0x73:
			replaceme(gs7070->replace73, buffer);
			break;
		default:
			;
	};
}

/* Allocate a regster and setup some values */
GS7070ControlReg* allocategs7070ctlreg(char matchhi, char matchlow,
								char replacehi, char replacelow)
{
	GS7070ControlReg* rp=(GS7070ControlReg*)malloc(sizeof(GS7070ControlReg));

	rp->count=0;
	rp->match[0]=matchhi;
	rp->match[1]=matchlow;
	rp->replace[0]=replacehi;
	rp->replace[1]=replacelow;
	return(rp);
}

/* Clean up a register */
void deallocategs7070ctlreg(GS7070ControlReg* gscr)
{
	free(gscr);
}

/* Allocate the gs7070 interrupt handler and do some initilisation */
void allocateGS7070int(void)
{
	/*which way round should a malloc be?*/
	gs7070=(GS7070ControlINT*)malloc(sizeof(GS7070ControlINT));

	gs7070->controlcodecount=-1;
	gs7070->replace73=allocategs7070ctlreg(0x73, 0x11, 0x63, 0x01);
	gs7070->replace53=allocategs7070ctlreg(0x53, 0x11, 0x43, 0x01);
	gs7070->controlseqcount=0;
	gs7070->count7341=0;
	gs7070->count734D=0;
}

/* Deallocate the gs7070 interrupt handler and clean up */
void deallocateGS7070int(){
	deallocategs7070ctlreg(gs7070->replace73);
	gs7070->replace73=0;
	deallocategs7070ctlreg(gs7070->replace53);
	gs7070->replace53=0;
	gs7070->controlcodecount=-1;
	free(gs7070);
}

/* get structure for aal5 header */
void getAal5HeaderStructure7070(const unsigned char* aal5Header,
                                struct aal5_header_st* aal5HeaderOut)
{
	aal5HeaderOut->vpi = ( aal5Header[0] <<  4) | (aal5Header[1] >> 4);
	aal5HeaderOut->vci = ((aal5Header[1] & 0x0f) << 12) | (aal5Header[2] << 4) | (aal5Header[3] >> 4);
	aal5HeaderOut->pti = ( aal5Header[3] & 0x0f);
}
