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
		printf("OEC.C - eco_init - START\n");
		eoc_out_buffer_pos = eocmescnt = eocmesval = eocstate = 0;
		printf("OEC.C - eco_init - END\n");
}

/*
 *		Decode the buffer and return the 13 bit eoc code  
 */
u_int16_t eoc_decode(unsigned char b1, unsigned char b2) {
	printf("OEC.C - eco_decode - START [b1b2 : %02x%02x]\n", b1, b2);
	printf("OEC.C - eco_decode - END   [eoc  : %04x]\n", (((b1 >>2) & 0x3f ) | ((b2 << 5) & 0x1fc0)));
	return (((b1 >>2) & 0x3f ) | ((b2 << 5) & 0x1fc0));
}

/*
 * Handle the oec messages in idle state
 */
void eoc_execute(u_int16_t eocmesval) {
	printf("OEC.C - eco_execute - START [eocmesval : %04x]\n", eocmesval);
	switch(EOC_OPCODE(eocmesval)) {
		case EOC_OPCODE_READ_0:
			eocstate = _preread;
			printf("OEC.C - eco_execute - STEP1 [eocmesval : EOC_OPCODE_READ_0]\n");
			eocreadpar = eocreadcnt = eocreadpos = 0;
			eocreadlen = 8;
			break;
		case EOC_OPCODE_READ_1:
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_1]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = 0;
			eocreadlen = 2;
			break;			
	}
	printf("OEC.C - eco_execute - END   [eocmesval : %04x]\n", eocmesval);
}

/*
 * Handle the eoc next opcode depending on the current state	
 */

int eoc_read_next() {
	int	mes;
	
	printf("OEC.C - eco_readnext - START [eocreadpos : %d| eocreadlen : %d]\n", eocreadpos, eocreadlen);
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
	printf("OEC.C - eco_readnext - END   [mes : %04x]\n", mes);
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
	
	printf("OEC.C - parse_eoc_buffer - START [eocmesval : %04x]\n", eocmesval);
	
	for(;i<bufflen;i+=2) {		
		if(buffer[i] !=0xc0) {
			/* check for eoc valicode*/
			eoccode = eoc_decode(buffer[i], buffer[i+1]);
			if((eoccode & 0x0F01) != 0x0301) continue;
			printf("OEC.C - parse_eoc_buffer - GOOD EOC CODE [%04x]\n", eoccode);
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
			printf("OEC.C - parse_eoc_buffer - CYCLE1 [eocmesval : %04x| eoccode . %04x| eocmescnt : %d| eocstate : %d| EOC_ADDRESS(eoccode) : %d]\n", eocmesval, eoccode, eocmescnt, eocstate, EOC_ADDRESS(eoccode));
			switch(eocstate) {
				case _preread:
						printf("OEC.C - parse_eoc_buffer - PREREAD [eocstate : _preread]\n");
						switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								printf("OEC.C - parse_eoc_buffer - PREREAD - [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
								if((eocmescnt >= 2) && (EOC_PARITY(eocmesval) == EOC_PARITY_ODD)) 
									eocstate = _read;
								if(eoc_out_buffer_pos < 30) { /* do the echo to ack it */
									eoc_out_buf[eoc_out_buffer_pos++] = (eocmesval & 0xff00) >> 8;
									eoc_out_buf[eoc_out_buffer_pos++] = eocmesval & 0x00ff;
								} else {
									return -EIO;
								}
								break;
							case EOC_OPCODE_RTN:
								printf("OEC.C - parse_eoc_buffer - PREREAD [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
							case EOC_OPCODE_HOLD:
								printf("OEC.C - parse_eoc_buffer - PREREAD [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
								if(eocmescnt >= 2) 
									eocstate = _idle;
								break;
						}
						if(eoc_out_buffer_pos < 30) { /* do the echo to ack it */
							printf("OEC.C - parse_eoc_buffer - PREREAD [eoc_out_buffer_pos < 30]\n");							
							eoc_out_buf[eoc_out_buffer_pos++] = (eocmesval & 0xff00) >> 8;
							eoc_out_buf[eoc_out_buffer_pos++] = eocmesval & 0x00ff;						} else {
							return -EIO;
						}
						break;
				case _idle:	/*like G992.2 recomendation */
					printf("OEC.C - parse_eoc_buffer - IDLE [eocstate : _idle]\n");
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
					printf("OEC.C - parse_eoc_buffer - READ [eocstate : _read]\n");
					switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								printf("OEC.C - parse_eoc_buffer - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
								mes = eoc_read_next();
								if(eoc_out_buffer_pos < 30) { /* do the echo to ack it */
									eoc_out_buf[eoc_out_buffer_pos++] = (mes & 0xff00) >> 8;
									eoc_out_buf[eoc_out_buffer_pos++] = mes & 0xff;
								} else {
									return -EIO;
								}
								break;
							case EOC_OPCODE_RTN:
								printf("OEC.C - parse_eoc_buffer - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
							case EOC_OPCODE_HOLD:
								printf("OEC.C - parse_eoc_buffer - CYCLE4C [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
								if(eocmescnt >= 2) 
									eocstate = _idle;
								break;
						}
						break;
			}
		}
		printf("OEC.C - parse_eoc_buffer - LOOP [eocmesval : %04x| eoccode . %02x| eocmescnt : %d| eocstate : %d| EOC_ADDRESS(eoccode) : %d]\n", eocmesval, eoccode, eocmescnt, eocstate, EOC_ADDRESS(eoccode));
	}
	printf("OEC.C - parse_eoc_buffer - END  [eocmesval : %04x]\n", eocmesval);
	return 0;
}

/* 
 * 	return the buffer for eoc answer
 */

 void get_oec_answer(unsigned char *eocoutbuff) {
 	int i;
 	
 	assert(eoc_out_buffer_pos<32);
 	printf("OEC.C - get_oec_answer - START [eoc_out_buffer_pos : %d]\n", eoc_out_buffer_pos);
	
 	for(i=0; i< 32; i++) {	/* to be optimized */
 			eocoutbuff[i] = 0x0c;
 	}
 	for(i=0; i < eoc_out_buffer_pos; i++) {
 		eocoutbuff[i] = eoc_out_buf[i++];
 	}
 	eoc_out_buffer_pos = 0;
	printf("OEC.C - get_oec_answer - END \n");
}

int has_eocs(){
	return (eoc_out_buffer_pos);
}
