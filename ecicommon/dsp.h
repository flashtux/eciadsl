/*
 *	Dsp.h
 * 
 *	Author : Valette Jean-S�bastien
 *
 * Creation : 30 09 2004
 * 
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
 * Describe the DSP on the host.
 * 
 * NOTE: Must be shareable by usermode and kernelmode drivers.
 * 
*/
#include <sys/types.h>
enum dsp_type {
		dsp_unknown = 0, dsp_7070, dsp_7470
};

struct gs7x70_dsp {
	dsp_type		type;
	unsigned char 	next_state;	/*	Next state for download operation */
	unsigned char 	is_ready;		/* Is the dsp OK ?	*/
	unsigned char 	last_frameid;	/*	ID of last frame received in int urb */
	u_int16_t		State;			/*	State ?	*/
	u_int16_t		StartProgress;	/*	Status of init progression	?*/
	u_int16_t		LinePower;		/*	Line power attenation.*/
};
