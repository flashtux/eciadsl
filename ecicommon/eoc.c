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

static int eocmescnt;			/*	Message counter */
static int eocmesval;			/*	last received message	*/
static eoc_state eocstate;		/*	State of the eoc system	*/
static int eocpar;				/*	1 odd, 0 even	*/
static int eocdataregpos;		/* position index in register	*/
static int eocreadlen;			/* length inread register	*/
static int eocwritelen;		/* length inread register	*/
static char *eocdatareg;		/*	pointer to data that will be read or write */


static unsigned char eoc_out_buf[32];	/* out buffer */
static int eoc_out_buffer_pos;

struct eoc_registers eocregs;

/*
 * 	init the eoc stuff
 */

void eoc_init() {
	    int i;
		printf("EOC.C - eco_init - START\n");
		eoc_out_buffer_pos = eocmescnt = eocmesval = eocstate = 0;
		for(i=0; i<8; i++)
			eocregs.vendorID[i] = 0x00;
		for(i=0; i<2; i++)
			eocregs.revision[i]	= 0x00;
		for(i=0; i<32; i++)
			eocregs.serial[i] 	= 0x00;
		eocregs.selftest[0] 	= 0x00;
		eocregs.vendor1[0]	 	= 0x00;
		eocregs.vendor2[0] 		= 0x00;
		eocregs.attenuation[0]	= 0x00;
		eocregs.SNRmargin[0]	= 0x00;
		for(i=0; i<30; i++)
			eocregs.ATURconfig[i]= 0x00;
		eocregs.linkstate	= 0x00;		
		printf("EOC.C - eco_init - END\n");
}

/*
 *		Decode the buffer and return the 13 bit eoc code  
 */
u_int16_t eoc_decode(unsigned char b1, unsigned char b2) {
	printf("EOC.C - eco_decode - START [b1 : %02x b2 : %02x]\n", b1, b2);
	printf("EOC.C - eco_decode - END     [b1: %04x b2 : %04x]\n", ((b1 >>2) & 0x3f ) , ((b2 << 5) & 0x1fc0));
	return (((b1 >>2) & 0x3f ) | ((b2 << 5) & 0x1fc0));
}

/*
 * Handle the eoc messages in idle state - changed by kolja
 */
void eoc_execute(u_int16_t eocmesval) {
	printf("EOC.C - eco_execute - START [eocmesval : %04x]\n", eocmesval);
	/*determine parity bit*/
	if((EOC_PARITY(eocmesval)>>3)!=eocpar){
		if(eocpar)
			eocpar=0;
		else
			eocpar=1;
		eocdataregpos++;
	}
	/*determine eocstate */	
	switch(eocstate){
		case _idle:
			switch(EOC_OPCODE(eocmesval)) {
				case EOC_OPCODE_READ_0:
					eocstate = _preread;
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_0]\n");
					eocdataregpos =eocmescnt = eocmesval  = 0;
					eocreadlen = 8;
					eocdatareg = &(eocregs.vendorID[0]);
					break;
				case EOC_OPCODE_READ_1:
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_1]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval  = 0;
					eocreadlen = 2;
					eocdatareg = &(eocregs.revision[0]);
					break;
				case EOC_OPCODE_READ_2:	/*	SERIAL  Number */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_2]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocreadlen = 32;
					eocdatareg = &(eocregs.serial[0]);
					break;
				case EOC_OPCODE_READ_3:	/*	Self test Result */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_3]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocreadlen = 1;
					eocdatareg = &(eocregs.selftest[0]);
					break;
				case EOC_OPCODE_READ_4:	/*	Vendor 1 */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_4]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval  = 0;
					eocreadlen = 1;
					eocdatareg = &(eocregs.vendor1[0]);
					break;
				case EOC_OPCODE_READ_5:	/*	Vendor 2 */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_5]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval  = 0;
					eocreadlen = 1;
					eocdatareg = &(eocregs.vendor2[0]);
					break;
				case EOC_OPCODE_READ_6:	/*	Attenuation */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_6]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval  = 0;
					eocreadlen = 1;
					eocdatareg = &(eocregs.attenuation[0]);
					break;
				case EOC_OPCODE_READ_7:	/*	SNR margin */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_7]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval  = 0;
					eocreadlen = 1;
					eocdatareg = &(eocregs.SNRmargin[0]);
					break;
				case EOC_OPCODE_READ_8:	/*	ATUR config */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_READ_8]\n");
					eocstate = _preread;
					eocdataregpos = eocmescnt = eocmesval  = 0;
					eocreadlen = 30;
					eocdatareg = &(eocregs.ATURconfig[0]);
					break;
				case EOC_OPCODE_WRITE_0:
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_0]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 8;
					eocdatareg = &(eocregs.vendorID[0]);
					break;
				case EOC_OPCODE_WRITE_1:
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_1]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 2;
					eocdatareg = &(eocregs.revision[0]);
					break;
				case EOC_OPCODE_WRITE_2:	/*	SERIAL  Number */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_2]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 32;
					eocdatareg = &(eocregs.serial[0]);
					break;
				case EOC_OPCODE_WRITE_3:	/*	Self test Result */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_3]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 1;
					eocdatareg = &(eocregs.selftest[0]);
					break;
				case EOC_OPCODE_WRITE_4:	/*	Vendor 1 */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_4]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 1;
					eocdatareg = &(eocregs.vendor1[0]);
					break;
				case EOC_OPCODE_WRITE_5:	/*	Vendor 2 */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_5]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 1;
					eocdatareg = &(eocregs.vendor2[0]);
					break;
				case EOC_OPCODE_WRITE_6:	/*	Attenuation */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_6]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 1;
					eocdatareg = &(eocregs.attenuation[0]);
					break;
				case EOC_OPCODE_WRITE_7:	/*	SNR margin */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_7]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 1;
					eocdatareg = &(eocregs.SNRmargin[0]);
					break;
				case EOC_OPCODE_WRITE_8:	/*	ATUR config */
					printf("EOC.C - eco_execute - IDLE [eocmesval : EOC_OPCODE_WRITE_8]\n");
					eocstate = _prewrite;
					eocdataregpos = eocmescnt = eocmesval = 0;
					eocwritelen = 30;
					eocdatareg = &(eocregs.ATURconfig[0]);
					break;
				default:
					eoc_encode(EOC_OPCODE_UTC);
					break;
			}
			break;
		case _preread:
			switch(EOC_OPCODE(eocmesval)) {
				case EOC_OPCODE_NEXT:
					printf("EOC.C - eco_execute - PREREAD - [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
					if(EOC_PARITY(eocmesval) == EOC_PARITY_ODD)
						eocstate = _read;
					break;
				case EOC_OPCODE_RTN:
					printf("EOC.C - eco_execute - PREREAD [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
				case EOC_OPCODE_HOLD:
					printf("EOC.C - eco_execute - PREREAD [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
					eocstate = _idle;
					break;	
			}
			break;
		case _prewrite:
			switch(EOC_OPCODE(eocmesval)) {
				case EOC_OPCODE_NEXT:
					printf("EOC.C - eco_execute - PREWRITE - [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
					if(EOC_PARITY(eocmesval) == EOC_PARITY_ODD)
						eocstate = _write;
					break;
				case EOC_OPCODE_RTN:
					printf("EOC.C - eco_execute - PREWRITE [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
				case EOC_OPCODE_HOLD:
					printf("EOC.C - eco_execute - PREWRITE [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
					eocstate = _idle;
					break;
			}
			break;
		case _read:
			switch(EOC_OPCODE(eocmesval)) {
				case EOC_OPCODE_RTN:
					printf("EOC.C - eco_execute - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
				case EOC_OPCODE_HOLD:
					printf("EOC.C - eco_execute - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
					eocstate = _idle;
					break;
			}
			break;
	}					
	printf("EOC.C - eco_execute - END   [eocmesval : %04x]\n", eocmesval);
}

/*
 * Handle the eoc next opcode depending on the current state	
 */

int eoc_read_next() {
	u_int16_t data;
	u_int16_t mes;
	
	printf("EOC.C - eco_readnext - START [eocdataregpos : %d| eocreadlen : %d]\n", eocdataregpos, eocreadlen);
	if(eocdataregpos < eocreadlen) {
		data = eocdatareg[eocdataregpos];
		mes = 0x4301;
	} else {
		mes = 0x5301;		
		data = 0x0e; /* EOD */
	}
	if(eocpar) {	/* set parity bit and switch eocreadpar value */
		mes |= EOC_ENCODED_PARITY_ODD;
	} else {
		mes |= EOC_ENCODED_PARITY_EVEN;
	}
	printf("EOC.C - eco_readnext - STEP1 [mes : %04x| data : %02x]\n", mes, data);
	mes |= (data & 0x01) << (7 + 8); /* 1st byte contain lsb in bit 7 */
	mes |= (data & 0xFE); /* 2d byte contain 7 msb in bits 1 to 7 */
	printf("EOC.C - eco_readnext - STEP2 [mes : %04x| data : %02x]\n", mes, data);
	printf("EOC.C - eco_readnext - END   [mes : %04x]\n", mes);
	return(mes);
}


void eoc_write_data(u_int16_t code) {
	printf("EOC.C - eco_writenext - START [eocdataregpos : %d| eocwritelen : %d]\n", eocdataregpos, eocwritelen);
	if(eocdataregpos < eocwritelen) {
		eocdatareg[eocdataregpos] = (code >> 5) & 0xff;
	}
	printf("EOC.C - eco_writenext - END   [code : %04x]\n", code);
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
	1111 0011 0100 1111 0xF34F -  op read_7 !
	1111 0011 0001 0011	0xF313 -  op request test reg param ??
	1111 0011 0100 0011	0xF343 -  op read_1 !! fuck!!
	0101 0011 0001 0001 0x5311
	0100 0011 0000 0001 0x4301
	0111 0011 0001 0001 0x7311
*/		
	printf("EOC.C - parse_eoc_buffer - START [eocmesval : %04x]\n", eocmesval);
	
	for(;i<bufflen;i+=2) {		
		if(buffer[i] !=0xc0) {
			/* check for eoc valicode*/
			if((((buffer[i] << 8 ) | buffer[i+1]) & 0x0D01) != 0x0101) continue;
			eoccode = eoc_decode(buffer[i], buffer[i+1]);
			printf("EOC.C - parse_eoc_buffer - GOOD EOC CODE [eoccode : %04x| EOC_ADDRESS(eoccode) : %04x| eocpar %d]\n", eoccode, EOC_ADDRESS(eoccode), eocpar);	
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
			if(eocmescnt >= 2){
				/* determine ecostate & eoc parity (eocpar)- kolja */
				eoc_execute(eocmesval);				
				printf("EOC.C - parse_eoc_buffer - CYCLE1 [eocmesval : %04x| eoccode . %04x| eocmescnt : %d| eocstate : %d| EOC_ADDRESS(eoccode) : %d]\n", eocmesval, eoccode, eocmescnt, eocstate, EOC_ADDRESS(eoccode));
				/* do actions if needed - kolja */
				if(!(eocmesval & EOC_DATA_MASK)) { /* handle data */
					eoc_write_data(eocmesval);
					if(eocdataregpos == eocwritelen) { /* if last  tell it to atu-c */
						mes = 0x5301;
						mes |= (0x0e & 0x01) << (7 + 8); /* 1st byte contain lsb in bit 7 */
						mes |= (0x0e & 0xFE) << 1 ; /* 2d byte contain 7 msb in bits 1 to 7 */
						if(eocpar) {	/* set parity bit and switch eocwritepar value */
							mes |= EOC_ENCODED_PARITY_ODD;
						} else {
							mes |= EOC_ENCODED_PARITY_EVEN;
						}
						eoc_out_buf[eoc_out_buffer_pos-2] = mes & 0xff;
						eoc_out_buf[eoc_out_buffer_pos-1] = (mes >>8 ) & 0xff;					
					}
					continue;
				} /* else handle an opcode */
				/* you could use if statement instead switch ??? - kolja */
				switch(eocstate) {
					case _read:
						printf("EOC.C - parse_eoc_buffer - READ [eocstate : _read]\n");
						switch(EOC_OPCODE(eocmesval)) {
							case EOC_OPCODE_NEXT:
								printf("EOC.C - parse_eoc_buffer - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_NEXT]\n");
								mes = eoc_read_next();
								eoc_out_buf[eoc_out_buffer_pos-1] = mes & 0xff;
								eoc_out_buf[eoc_out_buffer_pos-2] = (mes >>8 ) & 0xff;
								break;
							case EOC_OPCODE_RTN:
								printf("EOC.C - parse_eoc_buffer - READ [EOC_OPCODE(eocmesval) : EOC_OPCODE_RTN]\n");
							case EOC_OPCODE_HOLD:
								printf("EOC.C - parse_eoc_buffer - CYCLE4C [EOC_OPCODE(eocmesval) : EOC_OPCODE_HOLD]\n");
								break;
						}
						break;
				}
			}
		}
		printf("EOC.C - parse_eoc_buffer - LOOP [eocmesval : %04x| eoccode . %02x| eocmescnt : %d| eocstate : %d| EOC_ADDRESS(eoccode) : %d]\n", eocmesval, eoccode, eocmescnt, eocstate, EOC_ADDRESS(eoccode));
	}
	printf("EOC.C - parse_eoc_buffer - END  [eocmesval : %04x]\n", eocmesval);
	return 0;
}

/* 
 * 	return the buffer for eoc answer
 */

 void get_eoc_answer(unsigned char *eocoutbuff) {
 	int i;
 	
 	assert(eoc_out_buffer_pos<32);
 	printf("EOC.C - get_eoc_answer - START [eoc_out_buffer_pos : %d]\n", eoc_out_buffer_pos);
	
 	for(i=0; i< 32; i++) {	/* to be optimized */
 			eocoutbuff[i] = 0x0c;
 	}
 	for(i=0; i < eoc_out_buffer_pos; i++) {
 		eocoutbuff[i] = eoc_out_buf[i];
 	}
 	eoc_out_buffer_pos = 0;
	printf("EOC.C - get_eoc_answer - END \n");
}

int has_eocs(){
	printf("EOC.C - has_eocs  [eoc_out_buffer_pos : %d]\n", eoc_out_buffer_pos);
	return (eoc_out_buffer_pos);
}
void eoc_encode(u_int16_t eoc_opcode) {
	u_int16_t mes;
		
	mes = 0x5301;
	mes |= (eoc_opcode & 0x01) << (7 + 8); /* 1st byte contain lsb in bit 7 */
	mes |= (eoc_opcode & 0xFE) << 1 ; /* 2d byte contain 7 msb in bits 1 to 7 */
	eoc_out_buf[eoc_out_buffer_pos-2] = mes & 0xff;
	eoc_out_buf[eoc_out_buffer_pos-1] = (mes >> 8) & 0xff;		
};
