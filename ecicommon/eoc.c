/*
 *	eoc.c
 * 
 *	Author : Valette Jean-S�astien
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
#include <sys/types.h>
#include <errno.h>

#include "eoc.h"

static int eocmescnt;	/*	Message counter */
static int eocmesval;	/*	last received message	*/
static eoc_state eocstate;	/*	State of the oec system	*/
static int eocreadpar;		/*	0 odd, 1 even	*/
static int eocreadcnt;		/* readcounter	*/	
static int eocreadpos;		/* position index in readed register	*/
static int eocreadlen;		/* length inread register	*/

static unsigned char eoc_out_buf[32];	/* out buffer */
static int eoc_out_buffer_pos;

struct eoc_registers eocregs;

/*
 * 	init the eoc stuff
 */

void eoc_init() {
		eoc_out_buffer_pos = eocmescnt = eocmesval = eocstate = 0;
}

/*
 *		Decode the buffer and return the 13 bit eoc code  
 */
u_int16_t eoc_decode(unsigned char b1, unsigned char b2) {
	return (((b2>>2) & 0x3f) || ((b1 & 0xfe) << 5));
}

/*
 * Handle the oec messages in idle state
 */
void eoc_execute(u_int16_t eocmesval) {
	switch(EOC_OPCODE(eocmesval)) {
		case EOC_OPCODE_READ_0:
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = 0;
			eocreadlen = 8;
			break;
		case EOC_OPCODE_READ_1:
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = 0;
			eocreadlen = 2;
			break;			
	}
}

/*
 * Handle the eoc next opcode depending on the current state	
 */

int eoc_read_next() {
	int	mes;
	
	if(eocreadpos < eocreadlen) {
		mes = 0x2301;
		if(eocreadpar) {
			mes |= EOC_PARITY_EVEN;
		} else {
			mes |= EOC_PARITY_ODD;
		}
		mes |= 0; /* should or the register here but with bit manipulation */
	} else {
		mes = EOC_OPCODE_EOD;
	}
	return(mes);
}

/*
	parse and handle eoc code from the buffer
	
*/

int parse_eoc_buffer(unsigned char *buffer, int bufflen) {
	int i=0;
	int mes; 
	u_int16_t eoccode;
	
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
				eocmescnt++;
			}
			switch(eocstate) {
				case _preread:
						switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								if((eocmescnt >= 2) && (EOC_PARITY(eocmesval) == EOC_PARITY_ODD)) 
									eocstate = _read;
								if(eoc_out_buffer_pos < 31) { /* do the echo to ack it */
									eoc_out_buf[eoc_out_buffer_pos++] = eocmesval;
								} else {
									return -EIO;
								}
								break;
							case EOC_OPCODE_RTN:
							case EOC_OPCODE_HOLD:
								if(eocmescnt >= 2) 
									eocstate = _idle;
								break;
						}
						if(eoc_out_buffer_pos < 31) { /* do the echo to ack it */
							eoc_out_buf[eoc_out_buffer_pos++] = eocmesval;
						} else {
							return -EIO;
						}
						break;
				case _idle:	/*like G992.2 recomendation */
					if(eocmescnt >=2) /* execute third time with same message */
						eoc_execute(eocmesval);
					if(eoc_out_buffer_pos < 30) { /* do the echo to ack it */
						eoc_out_buf[eoc_out_buffer_pos++] = (eocmesval & 0xff00) >> 8;
						eoc_out_buf[eoc_out_buffer_pos++] = eocmesval & 0x00ff;
					} else {
						return -EIO;
					}
					break;
				case _read:
					switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								mes = eoc_read_next();
								if(eoc_out_buffer_pos < 30) { /* do the echo to ack it */
									eoc_out_buf[eoc_out_buffer_pos++] = (mes & 0xff00) >> 8;
									eoc_out_buf[eoc_out_buffer_pos++] = mes & 0xff;
								} else {
									return -EIO;
								}
								break;
							case EOC_OPCODE_RTN:
							case EOC_OPCODE_HOLD:
								if(eocmescnt >= 2) 
									eocstate = _idle;
								break;
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

 void get_oec_answer(unsigned char *eocoutbuff) {
 	int i;
 	
 	assert(eoc_out_buffer_pos<32);
 	
 	for(i=0; i< 32; i++) {	/* to be optimized */
 			eocoutbuff[i] = 0x0c;
 	}
 	for(i=0; i < eoc_out_buffer_pos; i++) {
 		eocoutbuff[i] = eoc_out_buf[i++];
 	}
 	eoc_out_buffer_pos = 0;
}

int has_eocs(){
	return (eocmescnt);
}
