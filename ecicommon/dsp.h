/*
 *	Dsp.h
 * 
 *	Author : Valette Jean-Sébastien
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
#include <type.h>

struct gs7x70_dsp {
	unsigned char 	next_state;	/*	Next state for download operation */
	unsigned char 	is_ready;		/* Is the dsp OK ?	*/
	unsigned char 	last_frameid;	/*	ID of last frame received in int urb */
	u_int16t		State;			/*	State ?	*/
	u_int16t		StartProgress;	/*	Status of init progression	?*/
	u_int16t		LinePower;		/*	Line power attenation.*/
};
