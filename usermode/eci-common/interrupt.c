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
#define DEBUG 1

#include <assert.h>
#include <stdio.h>
#include "interrupt.h"

#ifdef DEBUG
	#define DBG_OUT printf
#else
	#define DBG_OUT(...)
#endif

static union ep_int_buf interrupt_buffer;

static int interrupt_buffer_pos = 0;

/*
 * 	Parse the status buffer in order to retreive info 
 *  and copy the memory from the modem dsp 
 * 
 */

inline void dsp_parse_status(unsigned char *frameid, unsigned char *status_buffer, struct gs7x70_dsp *dsp){
	if(dsp->last_frameid+1 != (unsigned char)*frameid) 
		DBG_OUT("DSP_PARSE_STATUS - Globespan Modem miss a frame \n");
	switch((unsigned char)*frameid) {
		case 1:
			dsp->State = status_buffer[0] | 
							(status_buffer[1] << 8);
			dsp->StartProgress = status_buffer[6] | 
							(status_buffer[7] << 8);
			dsp->LinePower = status_buffer[8] | 
							(status_buffer[9] << 8);	
			DBG_OUT("DSP_PARSE_STATUS - DSP State : %04x | DSP StartProgess : %04x | DSP LinePower : %04x\n", 
			dsp->State, dsp->StartProgress, dsp->LinePower);	
			break;				
		default:
			DBG_OUT("DSP_PARSE_STATUS - Not handled frame : %0x2\n", (unsigned char)*frameid);
	}
	dsp->last_frameid = (unsigned char)*frameid;
};

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
static inline int dsp_parse_interrupt_buffer(unsigned char *buffer, int buffer_size,
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
			dsp_parse_status(&interrupt_buffer.buffer.frameid, interrupt_buffer.buffer.status_buffer , dsp);
		}
	} else {
		if(buffer_size < INT_MIN_BUFFER_SIZE) return -EINVAL;
		switch(buffer[1]){
			case 0xF0:
				dsp->is_ready = (buffer[2] << 8 ) | buffer[3];
				dsp->next_state = (buffer[4] << 8 )| buffer[5];
				DBG_OUT("DSP READY %d Next state %2x\n", dsp->is_ready, 
						dsp->next_state);
				break;
			default:
				break;
		}
	}
	return 0;
}

