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

#ifndef GS7470_H
#define GS7470_H

#include "gsinterface.h"
/*=============================================================
 *                    (kolja)
 *            COMMON DEVICE PARAMETERS 
 * (include also modem.h definitions - now it is deprecated)
 *============================================================*/

/* curent connections values */
#define CUR_CONN_UNKNOWN -1
#define CUR_CONN_F343	0
#define CUR_CONN_7341	1
#define CUR_CONN_734D	2
#define CUR_CONN_F34F   3

/*Global variable andd structure definition*/
typedef struct  /*GS7470ControlReg*/
{
	int count;
	int curRepPosition;
	int maxRepPosition;
	unsigned char* replace;
} GS7470ControlReg; /*GS7470ControlReg;*/

/* Structure to store the information about the current state of the the GS7470 */
typedef struct /* GS7470ControlINT */
{
	/* Current control codes.
	   this should -> registers but I don't know hat they are at the moment
	*/
	unsigned short controlcodes[CONTROLCODEBUFFERSIZE];
	unsigned int controlcodecount; /* how many cuntrol codes on in the buffer */
	unsigned int controlseqcount;	
	GS7470ControlReg* curReplace73; /* current 0x7311 sequence position replacement*/
	GS7470ControlReg* curReplace53; /* current 0x5311 sequence position replacement*/
	/* Just incase we need information about the current connection */
	unsigned int currentConnection;
} GS7470ControlINT; /* GS7470ControlINT */

void gs7470InitParams(void);
static inline void getAal5HeaderStructure7470(const unsigned char* aal5Header,
                                struct aal5_header_st* aal5HeaderOut);

#endif
