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


struct gs707x_dsp {
	unsigned char next_state;	/*	Next state for download operation */
	unsigned char is_ready;		/* Is the dsp OK ?	*/
};
