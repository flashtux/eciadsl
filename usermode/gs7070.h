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
*/

#ifndef GS7070_H
#define GS7070_H

/*=============================================================
 *                         (kolja)
 *                 COMMON DEVICE PARAMETERS 
 * (include also modem.h definitions - now it is deprecated)
 *============================================================*/

typedef struct  /* GS7070ControlReg */
{
/* nood to think of a better name for this, it's really
  the GS7070 int control maybe GSControlReg*/
	int count;
	unsigned char match[2];
	unsigned char replace[2];
} GS7070ControlReg; /* GS7070ControlReg */

/* Structure to store the information about the current state of the the S7070 */

typedef struct /* GS7070ControlINT */
{
	/* Current control codes.
	   this should -> registers but I don't know hat they are at the moment
	*/
	unsigned short controlcodes[CONTROLCODEBUFFERSIZE];
	int controlcodecount; /* how many cuntrol codes on in the buffer */
	int controlseqcount;	
	GS7070ControlReg* replace73; /* what sequence possition are we at (for 73's) */
	GS7070ControlReg* replace53; /* what sequence possition are we at (for 53's) */
	/* Just incase we need information about the current connection */
	int count734D;
	int count7341;
	int sequencecount; /* not used, I have seperate counters for different sequences at the moment! */
	short vid; /* Modem vendor/product codes not in use at the moment */
	short pid;
	short vci;
	short vpi;	
	short annax;
} GS7070ControlINT; /* GS7070ControlINT */

void gs7070InitParams(void);
int gs7070SetControl(unsigned char* buffer);
void gs7070GetResponse(unsigned char* buffer);
GS7070ControlReg* allocategs7070ctlreg(char matchhi, char matchlow, char replacehi, char replacelow);
void deallocategs7070ctlreg(GS7070ControlReg* gscr);
void allocateGS7070int(void);
void deallocateGS7070int(void);
void getAal5HeaderStructure7070(const unsigned char* aal5Header,
                                struct aal5_header_st* aal5HeaderOut);

#endif
