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
static eoc_state eocstate;	/*	State of the oec system	*/
static int eocreadpar;		/*	0 odd, 1 even	*/
static int eocreadcnt;		/* readcounter	*/	
static int eocreadpos;		/* position index in readed register	*/

static char eoc_out_buf[32];	/* out buffer */
static int eoc_out_buffer_pos;

struct eoc_registers eocregs;

/*
 * 	init the eoc stuff
 */

void eoc_init() {
		eoc_out_buffer_pos = eocmescnt = eocmesval = eocstate = 0;
};

/*
 *		Decode the buffer and return the 13 bit eoc code  
 */
u_int16t eoc_decode(char b1, char b2) {
	return (((b2>>2) & 0x3f)) || ((b1 & 0xfe) << 5));
}

/*
 * Handle the oec messages in idle state
 */
void eoc_execute(u_int16_t eocmesval) {
	switch(EOC_OPCODE(eocmesval)) {
		case EOC_OPCODE_READ_0:
			eocstate = preread;
			eocreadpar = oecreadcnt = eocreadpos = 0;
			eocreadlen = 8;
			break;
		case EOC_OPCODE_READ_1:
			eocstate = preread;
			eocreadpar = ecreadcnt = eocreadpos = 0;
			eocreadlen = 2;
			break;			
		default:
			
			/* break; useless	*/
	}
}

/*
 * Handle the eoc next opcode depending on the current state	
 */

void eoc_next() {
	int	mes;
	
	switch(eocstate) {
		case preread:
			eocstate = read;
		break;
		case read:
			if(eocreadpos < eocreadlen) {
				mes=0;
				mes | = 0 ;		
			} else {
				mes = EOC_OPCAODE_REP_EOD;
			}
		break;
	}
}

/*
	parse and handle eoc code from the buffer
	
*/

int parse_eoc_buffer(char *buffer, int bufflen) {
	int i=0; 
	int err
	u_int16t eoccode;
	
	assert(bufflen < EOC_MAX_LEN);
	
	for(;i<bufflen;i+=2) {
		if(buffer[i] !=0xc0) {
			/* TODO: check for eoc valicode*/
			eoccode = eoc_decode(buffer[i], buffer[i+1]);
			if(EOC_ADDRESS(eoccode) != EOC_ADDRESS_ATU_R) {
				continue; /* creapy message or not for us */
			}
			if(eocmesval != eoccode) { /* new message */
				eocmesval = eoccode;
				eocmescnt = 0;
				continue;
			} else {
				oecmescnt++;
			}
			switch(eocstate) {
				case preread:
						switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								if((eocmescnt >= 2) && (EOC_PARITY == EOC_PARITY_ODD)) 
									eocstate = read;
								if(eoc_out_buffer_pos < 31) { /* do the echo to ack it */
									eoc_out_buff[eoc_out_buffer_pos++] = eocmesval;
								} else {
									return -EIO;
								}
								break;
							case RTN:
							case HOLD:
								if(eocmescnt >= 2) 
									eocstate = idle;
								break;
						}
						if(eoc_out_buffer_pos < 31) { /* do the echo to ack it */
							eoc_out_buff[eoc_out_buffer_pos++] = eocmesval;
						} else {
							return -EIO;
						}
						break;
				case idle:	/*like G992.2 recomendation */
					if(eocmescns >=2) /* execute third time with same message */
						if((err = eoc_execute(eocmesval)) <0) return err;
					if(eoc_out_buffer_pos < 31) { /* do the echo to ack it */
						eoc_out_buff[eoc_out_buffer_pos++] = eocmesval;
					} else {
						return -EIO;
					}
					break;
			}
		}				
	}
	return 0;
}

/* 
 * 	return the buffer for eoc answer
 */

 void get_oec_answer(char *eocoutbuff) {
 	int i;
 	
 	assert(eoc_out_buffer_pos<32);
 	
 	for(i = 0; i< 32; i++) {	/* to be optimized */
 			eocoutbuff[i] = 0x0c;
 	}
 	for(i=0; i < eoc_out_buffer_pos; i++) {
 		eocoutbuff[i] = eoc_out_buf[i++];
 	}
 	eoc_out_buffer_pos = 0;
}
