/*
 *	eciadsl-ctrlui.c
 * 
 *	Author : kolja gavaATbergamoblog.it
 *
 * Creation : 40 05 2005
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
 * ECI UI CONTROL PROCESS
 * 
 * this process can handle entire modem process, at the moment it's used to :
 *    -- disconnect
 * 	  -- force eci msg queue deallocation
 * 
 */
 
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include "ecimsgh.h"

struct eci_msg ecimsg;

void do_disconnect(){
	printf ("Please wait... disconnecting\n");
	sndEciMsg(ECI_MC_DO_DISCONNECT, NULL, 0, ECI_PT_PPP, 1);
	rcvEciMsg(&ecimsg,1,0);
	if (ecimsg.ecicmd == ECI_MC_DISCONNECTED)
		printf ("modem disconnected\n");
}

void force_cleanup_queue(){
	int ret=0;
	printf ("Please wait... deallocating queue\n");
	initEciMsgQueue(0);
		
	while (ret>0 && errno!=ENOMSG){			
		ret = rcvEciMsg(&ecimsg,0,1);
	}
	initEciMsgQueue(ECI_PT_UI);
	printf ("deallocation queue succeeded\n");
}


int main(int argc, char** argv){
	int i;
	initEciMsgQueue(ECI_PT_UI);

	/* parse command line options */
	for (i = 1; i < argc; i++){
		if (strcmp(argv[i], "--disconnect") == 0){
			do_disconnect();
		}
			
		else 
		if (strcmp(argv[i], "--force-dealloc") == 0){
			force_cleanup_queue();
		}
	}
	deallocEciMsgQueue();
	return(0);
}
