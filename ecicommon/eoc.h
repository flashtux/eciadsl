/*
 *	eoc.h
 * 
 *	Author : Valette Jean-Sébastien
 *
 * Creation : 20 01 2004
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

#define EOC_MAX_LEN 34

#define EOC_ADDRESS(x) (x & 3)
#define ECO_ADDRESS_ATU_R 0
#define EOC_ADDRESS_ATU_C 3

#define EOC_OPCODE(x)		(x & (0xFF << 5)
#define EOC_OPCODE_HOLD		(0x01 << 5)
#define EOC_OPCODE_RTN		(0xf0 << 5)
#define EOC_OPCODE_SLFTST	(0x02 << 5)
#define EOC_OPCODE_REQCOR	(0x07 << 5)
#define EOC_OPCODE_REQEND	(0x08 << 5)
#define EOC_OPCODE_NOTCOR	(0x0b << 5)
#define EOC_OPCODE_NOTEND	(0x0d << 5)
#define EOC_OPCODE_EOD		(0x0e << 5)
#define EOC_OPCODE_REQTPU	(0x13 << 5)
#define EOC_OPCODE_GNTPDN	(0x16 << 5)
#define EOC_OPCODE_REJPDN	(0x83 << 5)
#define EOC_OPCODE_NEXT		(0x10 << 5)
#define EOC_OPCODE_READ_0	(0x20 << 5)
#define EOC_OPCODE_READ_1	(0x23 << 5)
#define EOC_OPCODE_READ_2	(0x25 << 5)
#define EOC_OPCODE_READ_3	(0x26 << 5)
#define EOC_OPCODE_READ_4	(0x29 << 5)
#define EOC_OPCODE_READ_5	(0x2A << 5)
#define EOC_OPCODE_READ_6	(0x2C << 5)
#define EOC_OPCODE_READ_7	(0x2F << 5)
#define EOC_OPCODE_READ_8	(0x31 << 5)
#define EOC_OPCODE_READ_9	(0x32 << 5)
#define EOC_OPCODE_READ_A	(0x34 << 5)
#define EOC_OPCODE_READ_B	(0x37 << 5)
#define EOC_OPCODE_READ_C	(0x38 << 5)
#define EOC_OPCODE_READ_D	(0x3B << 5)
#define EOC_OPCODE_READ_E	(0x3E << 5)
#define EOC_OPCODE_READ_F	(0x3F << 5)
 


enum eoc_state { idle =0, utc, exe, preread, read, prewirte, write);

struct eoc_registers {
	char vendorID[8];	/* reg 0 */
	char revision[2];	/* reg 1 */
	char serial[32]; 	/* reg 2 */
	char slefttest[1];	/* reg 3 */
	char vendor1[1];	/* reg 4 */
	char vendor2[1];	/* reg 5 */
	char attenuation[1];/* reg 6 */
	char SNRmmargin[1];	/* reg 7 */
	char ATURconfig[30];/* reg 8 */
						/* no reg 9 reserved for future */
	char linkstate;		/* reg A */
						/* no reg B to F reserved for future */
}