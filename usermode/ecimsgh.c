/*
 *	ecimsgh.c
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
 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h> 
#include <stdio.h>
#include "ecimsgh.h"
#include "util.h"

#ifdef DEBUG
	#define DBG_OUT printf
#else
	#define DBG_OUT(...)
#endif

/*queue id */
int iMsgQueueId=-1;

/*msg structure*/
struct {
	long 			mtype;
	struct eci_msg  mtext;
} msgbuf;

/* process calling for queue Messages*/
long lProcessType = (long)ECI_PT_UNKNOWN;

/*return message queue id and create it if it still does not set*/
int getMsgQueueId(){
	/*check if message queue id is already setted */
	if(iMsgQueueId==-1){
		key_t kEciKey;
		kEciKey = ftok(config_filename(), 'X');
		DBG_OUT ("**** kEciKey : %i\n", kEciKey);
		DBG_OUT ("**** errno : %i\n", errno);		
		iMsgQueueId = msgget(kEciKey, 0666);
		/*queue does not exists -> create it */
		if (iMsgQueueId<0 && errno==ENOENT){
			DBG_OUT ("**** id non settato kEciKey : %i\n", kEciKey);
			iMsgQueueId = msgget(kEciKey, IPC_CREAT | 0666);	
		}	
	}
	DBG_OUT ("**** iMsgQueueId : %d\n", iMsgQueueId);
	DBG_OUT ("**** errno : %i\n", errno);		
	return (iMsgQueueId);
}

int rcvEciMsg(struct eci_msg* ecimsg, int wait, int force_read){
	ssize_t retval;
	int msgflg=0;

	if (!wait)
		msgflg|= IPC_NOWAIT;
	if (force_read)
		msgflg|= MSG_NOERROR;

	retval = msgrcv(getMsgQueueId(), &msgbuf, sizeof(msgbuf.mtext), lProcessType, msgflg);
	
	if(retval>0){
		memcpy(ecimsg, &(msgbuf.mtext), sizeof(msgbuf.mtext));
		DBG_OUT ("*** received %i\n", msgbuf.mtext.ecicmd);
		DBG_OUT ("*** received sender %i\n", msgbuf.mtext.sender);		
	}else{
		ecimsg = NULL;
	}
	return((int)retval);
}

int sndEciMsg(enum EciadslMsgCmd msgcmd, char * data, int datasize, long DestEciPT, int wait){
	int retval;
	int i;
	int msgflg=0;

	if (!wait)
		msgflg|= IPC_NOWAIT;
		
	msgbuf.mtype = DestEciPT;
	msgbuf.mtext.sender=lProcessType;
	msgbuf.mtext.ecicmd = msgcmd;
	if (datasize>0){
		for (i=0;i<datasize;i+=ECI_MSG_MAX_DATA_SIZE){
			msgbuf.mtext.datasize= datasize;	
			memcpy(&(msgbuf.mtext.data), data+i, ECI_MSG_MAX_DATA_SIZE);
			retval = msgsnd(getMsgQueueId(), &msgbuf, sizeof(msgbuf.mtext), msgflg);
			if (retval<0){
				return(retval);
			}
		}
	}else{
		msgbuf.mtext.datasize= datasize;
		retval = msgsnd(getMsgQueueId(), &msgbuf, sizeof(msgbuf.mtext), msgflg);
	}
	return(retval);
}


int initEciMsgQueue(long EciPT){
	lProcessType = EciPT;
	return(0);
}

/* ATTENTION : process initializing EciMsgQueue must call this function on exit 
 * 			   remember to handle all termination signal (at least : SIGTERM, SIGINT, SIGQUIT)
 */
int deallocEciMsgQueue(){
	struct msqid_ds queueInfo;
	int retval=0;
	
	retval = msgctl(getMsgQueueId(), IPC_STAT, &queueInfo);
	if (retval<0)
		return(retval);

		DBG_OUT ("stat %i\n", queueInfo.msg_qnum);
		
	if (queueInfo.msg_qnum==0){
		retval = msgctl(getMsgQueueId(), IPC_RMID, &queueInfo);
		DBG_OUT ("closed %i\n", retval);		
		iMsgQueueId=-1;
	}
	return(retval);	
}
