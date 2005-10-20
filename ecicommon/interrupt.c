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

inline void dsp_parse_status(union ep_int_buf* buffer,  struct gs7x70_dsp *dsp){
	static unsigned char frameid;
	static unsigned char *status_buffer;
	switch(dsp->type){
		case dsp_7070:
			frameid = buffer->buffer_7070.frameid;
			status_buffer = &(buffer->buffer_7070.status_buffer[0]);
			break;
		case dsp_7470:
			frameid = buffer->buffer_7470.frameid;
			status_buffer = &(buffer->buffer_7470.status_buffer[0]);
			break;
		case dsp_unknown:
			DBG_OUT("Not initialised dsp value !!!\n");
			return;
			break;
		default:
			DBG_OUT("What the hell  is dsp number  %4x",dsp->type);
	}
	if(dsp->last_frameid+1 != frameid) 
		DBG_OUT("DSP_PARSE_STATUS - Globespan Modem miss a frame \n");
	switch(frameid) {
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
/* frameid 4 has some intreresting things on line cable disconnection 
 * it could be used to determine status ? */
/*		case 4:
			dsp->State = status_buffer[2] | 
							(status_buffer[3] << 8);*/							
		default:
			DBG_OUT("DSP_PARSE_STATUS - Not handled frame : %0x2\n", frameid);
	}
	dsp->last_frameid = frameid;
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
inline int dsp_parse_interrupt_buffer(unsigned char *buffer, int buffer_size,
							struct gs7x70_dsp *dsp){

	memcpy (&(interrupt_buffer.raw[interrupt_buffer_pos]), 
									buffer, INT_MAXPIPE_SIZE);	
	interrupt_buffer_pos += buffer_size;
	if((buffer_size == INT_MAXPIPE_SIZE) && (buffer[0] == 0xf0)){
		if(interrupt_buffer_pos == sizeof(union ep_int_buf)) {
			interrupt_buffer_pos = 0;
/*	TODO : Handle the eoc buffer !!!! */
			dsp_parse_status(&interrupt_buffer, dsp);
		}
	} else {
		if(buffer_size < INT_MIN_BUFFER_SIZE) return -EINVAL;
		switch(interrupt_buffer.small_buffer.Notification){
			case 0xF0:
				dsp->is_ready = (buffer[2] << 8 ) | buffer[3];
				dsp->next_state = (buffer[4] << 8 )| buffer[5];
				DBG_OUT("DSP READY %d Next state %2x\n", dsp->is_ready, 
						dsp->next_state);
				DBG_OUT("DSP Notification\nIsReady %d\n NextState %4x",
						interrupt_buffer.small_buffer.Value,			
						interrupt_buffer.small_buffer.Index);								
				break;
			default:
				DBG_OUT("DSP - Invalid Notification %2x\n", interrupt_buffer.small_buffer.Notification);
				break;
		}
		interrupt_buffer_pos = 0;
	}
	return 0;
}

