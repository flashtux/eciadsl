/*
 *	Interupt.c
 * 
 *	Author : Valette Jean-Sébastien
 *
 * Creation : 30 09 2004
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
 * 
 * NOTE : this code is for both usermode and kernelmode so be careful when 
 * 		  using stdlib call.
*/

#include "interrupt.h"

static unsigned char interrupt_buffer[INT_BUFFER_LEN];

static int interrupt_buffer_pos = 0;

/*
 *	Parse the buffer and  return zero if OK, -ERRSOMETHING else.
 * 	When ok, the resp buffer is set with the data that need to be 
 *  sent to the modem and *resp_size is the len of these data.
 * 
*/
int parse_interrupt_buffer(unsigned char *buffer, int buffer_size,
							unsigned char *resp, int *resp_size)
{
	int max_resp_size;
	
	max_resp_size = *resp_size;
	if(buffer_size == INT_MAXPIPE_SIZE)	{
		if(interrupt_buffer_pos == 0) {
			memcpy (&(interrupt_buffer[0]), buffer, INT_MAXPIPE_SIZE);
		}
	} else {
		if(buffer_size < INT_MIN_BUFFEr_SIZE) return -EINVAL;
		switch(buffer[1])
		{
			case 0xF0:
				break;
			default:
				break;
		}
	}
	return 0;
}
