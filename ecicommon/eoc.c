/*
 *	eoc.c
 * 
 *	Author : Valette Jean-Sébastien
 *
 * Creation : 18 01 2004
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
 * Handle the eoc buffer
 * 
 * NOTE : this code is for both usermode and kernelmode so be careful when 
 * 		  using stdlib call.
*/
#include <assert.h>

#include "eoc.h"

static int eocmescnt;	/*	Message counter */
static int eocmesval;	/*	last received message	*/
static int eocstate;	/*	State of the oec system	*/


/*
 * 
 */

void eoc_init() {
		eocmescnt = eocmesval = eocstate = 0;
};

/*
 *		Decode the buffer and return the 13 bit eoc code  
 */
u_int16t eoc_decode(char b1, char b2) {
	return (((b2>>2) & 0x3f)) || ((b1 & 0xfe) << 5));
}
/*
	parse and handle eoc code from the buffer
	
*/

int parseEocBuffer(char *buffer, int bufflen) {
	int i=0; 
	u_int16t eoccode;
	
	assert(bufflen < EOC_MAX_LEN);
	
	for(;i<bufflen;i+=2) {
		if(buffer[i] !=0xc0) {
			/* TODO: check for eoc valicode*/
			eoccode = eoc_decode(buffer[i], buffer[i+1]);
			if(EOC_ADDRESS(eoccode) != EOC_ADDRESS_ATU_R) {
				continue; /* creapy message or not for us */
			}
		}
	}
}