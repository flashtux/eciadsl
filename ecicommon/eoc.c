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
static int eocwritepar;		/*	0 odd, 1 even	*/
static int eocwritecnt;		/* readcounter	*/	
static int eocwritepos;		/* position index in readed register	*/
static int eocwritelen;		/* length inread register	*/
static char *eocnextrw;		/*	pointer to data that will be read or write */


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
	printf("OEC.C - eco_decode - START [b1 : %02x b2 : %02x]\n", b1, b2);
	printf("OEC.C - eco_decode - END     [b1: %04x b2 : %04x]\n", ((b1 >>2) & 0x3f ) , ((b2 << 5) & 0x1fc0));
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
			eocreadpar = eocreadcnt = eocreadpos =eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 8;
			eocnextrw = &(eocregs.vendorID[0]);
			break;
		case EOC_OPCODE_READ_1:
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_1]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 2;
			eocnextrw = &(eocregs.revision[0]);
			break;
		case EOC_OPCODE_READ_2:	/*	SERIAL  Number */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_2]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 32;
			eocnextrw = &(eocregs.serial[0]);
			break;
		case EOC_OPCODE_READ_3:	/*	Self test Result */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_3]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 1;
			eocnextrw = &(eocregs.selftest[0]);
			break;
		case EOC_OPCODE_READ_4:	/*	Vendor 1 */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_4]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 1;
			eocnextrw = &(eocregs.vendor1[0]);
			break;
		case EOC_OPCODE_READ_5:	/*	Vendor 2 */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_5]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 1;
			eocnextrw = &(eocregs.vendor2[0]);
			break;
		case EOC_OPCODE_READ_6:	/*	Attenuation */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_6]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 1;
			eocnextrw = &(eocregs.attenuation[0]);
			break;
		case EOC_OPCODE_READ_7:	/*	SNR margin */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_7]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 1;
			eocnextrw = &(eocregs.SNRmargin[0]);
			break;
		case EOC_OPCODE_READ_8:	/*	ATUR config */
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_READ_8]\n");
			eocstate = _preread;
			eocreadpar = eocreadcnt = eocreadpos = eocmescnt = eocmesval = eocstate = 0;
			eocreadlen = 30;
			eocnextrw = &(eocregs.ATURconfig[0]);
			break;
		case EOC_OPCODE_WRITE_0:
			printf("OEC.C - eco_execute - STEP2 [eocmesval : EOC_OPCODE_WRITE_0]\n");
			eocstate = _prewrite;
			oecwritepar = eocwritecnt = oecwritepos = eocmescnt = oecmesval = eocstate= 0;
			eocwritelen = 8;
			eocnextrw = &(eocregs.vendorID[0]);
			break;
	}
	printf("OEC.C - eco_execute - END   [eocmesval : %04x]\n", eocmesval);
}

/*
 * Handle the eoc next opcode depending on the current state	
 */

int eoc_read_next() {
	u_int16_t data;
	u_int16_t mes;
	
	printf("OEC.C - eco_readnext - START [eocreadpos : %d| eocreadlen : %d]\n", eocreadpos, eocreadlen);
	if(eocreadpos < eocreadlen) {
		data = *eocnextrw++;
		mes = 0x4301;
	} else {
		mes = 0x5301;		
		data = 0x0e; /* EOD */
	}
	if(eocreadpar) {	/* set parity bit and switch eocreadpar value */
		mes |= EOC_PARITY_EVEN;
		eocreadpar = 0;
	} else {
		mes |= EOC_PARITY_ODD;
		eocreadpar = 1;			
	}
	mes |= (data & 0x01) << (7 + 8); /* 1st byte contain lsb in bit 7 */
	mes |= (data & 0xFE) << 1 ; /* 2d byte contain 7 msb in bits 1 to 7 */
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
/*	          1111 
	6543 21xx 3210 987x
	0000 1111 0000 0001
	0000 0011 0000 0001

	0000 1101 0000 0001
	0000 0001 0000 0001
*/		
	printf("OEC.C - parse_eoc_buffer - START [eocmesval : %04x]\n", eocmesval);
	
	for(;i<bufflen;i+=2) {		
		if(buffer[i] !=0xc0) {
			/* check for eoc valicode*/
			if((((buffer[i] << 8 ) | buffer[i+1]) & 0x0D01) != 0x0101) continue;
			eoccode = eoc_decode(buffer[i], buffer[i+1]);
			printf("OEC.C - parse_eoc_buffer - GOOD EOC CODE [eoccode : %04x| EOC_ADDRESS(eoccode) : %04x]\n", eoccode, EOC_ADDRESS(eoccode));	
			if(EOC_ADDRESS(eoccode) != EOC_ADDRESS_ATU_R) {
				continue; /* creapy message or not for us */
			}
			eoc_out_buffer_pos+=2;
			if(eoc_out_buffer_pos < 33) { /* do the echo to ack it */
				eoc_out_buf[eoc_out_buffer_pos-2] = buffer[i];
				eoc_out_buf[eoc_out_buffer_pos-1] = buffer[i+1];
			} else {
				return -EIO;
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
								break;
							case EOC_OPCODE_RTN:
								printf("OEC.C - parse_eoc_buffer - PREREAD [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
							case EOC_OPCODE_HOLD:
								printf("OEC.C - parse_eoc_buffer - PREREAD [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
								if(eocmescnt >= 2) 
									eocstate = _idle;
								break;
						}
						break;
				case _prewrite:
					printf("EC.C - parse_eoc_buffer - PREWRITE[eocstate : _prewrite]\n");
					switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								printf("OEC.C - parse_eoc_buffer - PREWRITE - [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
								if((eocmescnt >= 2) && (EOC_PARITY(eocmesval) == EOC_PARITY_ODD)) 
									eocstate = _write;
								break;
							case EOC_OPCODE_RTN:
								printf("OEC.C - parse_eoc_buffer - PREWRITE [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
							case EOC_OPCODE_HOLD:
								printf("OEC.C - parse_eoc_buffer - PREWRITE [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
								if(eocmescnt >= 2) 
									eocstate = _idle;
								break;
						}
						break;
				case _idle:	/*like G992.2 recomendation */
					printf("OEC.C - parse_eoc_buffer - IDLE [eocstate : _idle]\n");
					if(eocmescnt >=2) /* execute third time with same message */
						eoc_execute(eocmesval);
					break;
				case _read:
					printf("OEC.C - parse_eoc_buffer - READ [eocstate : _read]\n");
					switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								printf("OEC.C - parse_eoc_buffer - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
								mes = eoc_read_next();
								eoc_out_buf[eoc_out_buffer_pos-2] = mes & 0xff;
								eoc_out_buf[eoc_out_buffer_pos-1] = (mess >>8 ) & 0xff;
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
 		eocoutbuff[i] = eoc_out_buf[i];
 	}
 	eoc_out_buffer_pos = 0;
	printf("OEC.C - get_oec_answer - END \n");
}

int has_eocs(){
	printf("OEC.C - has_eocs  [eoc_out_buffer_pos : %d]\n", eoc_out_buffer_pos);
	return (eoc_out_buffer_pos);
}
