/*
 *	Interupt.c
 * 
 *	Author : Valette Jean-S�bastien
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
 * Handle the dsp int urb buffers.
 * 
 * NOTE : this code is for both usermode and kernelmode so be careful when 
 * 		  using stdlib call.
*/

#include "interrupt.h"

static union ep_int_buf interrupt_buffer;

static int interrupt_buffer_pos = 0;

/*
 *	Parse the buffer and  return zero if OK, -ERRSOMETHING else.
 * 	When ok, the resp buffer is set with the data that need to be 
 *  sent to the modem and *resp_size is the len of these data.
 * 
 * TODO : MAY need to handle the down state for DSP (is_ready becoming 0)
 *        I think about returning a specifi -EXX to be conpatible with 
 *        both arch. But defining a common call should work too.
 * 
 * ASSUMPTION :INT_MAXPIPE_SIZE  divide the ep_int_buff struct size.
*/
int parse_interrupt_buffer(unsigned char *buffer, int buffer_size,
							unsigned char *resp, int *resp_size,
							struct gs7x70_dsp *dsp){
	int max_resp_size;
	
	max_resp_size = *resp_size;
	if((buffer_size == INT_MAXPIPE_SIZE) && (buffer[0] == 0xf0))	{
		memcpy (&(interrupt_buffer.raw[interrupt_buffer_pos]), 
									buffer, INT_MAXPIPE_SIZE);
		interrupt_buffer_pos += buffer_size;
		if(interrupt_buffer_pos == sizeof(union ep_int_buf)) {
			interrupt_buffer_pos = 0;
/*	TODO : Handle the buffer !!!! */
		}
	} else {
		if(buffer_size < INT_MIN_BUFFER_SIZE) return -EINVAL;
		switch(buffer[1]){
			case 0xF0:
				dsp->is_ready = (buffer[2] << 8 ) | buffer[3];
				dsp->next_state = (buffer[4] << 8 )| buffer[5];
				break;
			default:
				break;
		}
	}
	return 0;
}