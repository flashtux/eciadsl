/*
 *	ecimsgh.h
 * 
 *	Author : kolja gavaATbergamoblog.it
 *
 * Creation : 21 05 2005
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
 * eci messages Handler between eci process
 * 
 * ATTENTION : process initializing EciMsgQueue must call deallocEciMsgQueue function on exit.
 * 			   Remember to handle all termination signal (at least : SIGTERM, SIGINT, SIGQUIT)
 *
 */

#ifndef ECIMSGH_H
#define ECIMSGH_H

/*eciadsl process type list*/
enum EciadslProcessType{
	ECI_PT_SYNCH=0x0000000000000001,
	ECI_PT_PPP=0x0000000000000100,
	ECI_PT_UI=0x0000000000010000,
	ECI_PT_UNKNOWN=0x0000000001000000
};

/*eciadsl msg command list*/
enum EciadslMsgCmd{
	ECI_MC_NO_ACTION=0,
	ECI_MC_DO_FIRM=1,
	ECI_MC_FIRMED=2,
	ECI_MC_DO_SYNCH=3,
	ECI_MC_SYNCHED=4,
	ECI_MC_DO_CONNECT=5,
	ECI_MC_CONNECTED=6,
	ECI_MC_BANDWIDTH=7,
	ECI_MC_DO_DISCONNECT=8,
	ECI_MC_DISCONNECTED=9,
	ECI_MC_CMD_FAILED=10,
	ECI_MC_CONNECTION_FAILED=11
};

#define ECI_MSG_MAX_DATA_SIZE 10

struct eci_msg{
	long 				sender;	
	enum EciadslMsgCmd  ecicmd;
	unsigned short  	datasize;
	char 				data[ECI_MSG_MAX_DATA_SIZE];
};

int rcvEciMsg(struct eci_msg* ecimsg, int wait, int force_read);
int sndEciMsg(enum EciadslMsgCmd msgcmd, char * data, int datasize, long DestEciPT, int wait);
int initEciMsgQueue(long EciPT);
int deallocEciMsgQueue();

#endif
