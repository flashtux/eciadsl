/*
 *	eoc.h
 * 
 *	Author : Valette Jean-S�astien
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

#define EOC_ADDRESS(x) 		(x  & 0x0003)
#define EOC_ADDRESS_ATU_R 	0x0000
#define EOC_ADDRESS_ATU_C 	0x0003
#define EOC_DATA_MASK 		0x0004

#define EOC_DECODE_OP(x)	(x >> 5)
#define EOC_ENCODE_OP(x)	(x << 5)
#define EOC_OPCODE(x)		(x & (0xFF << 5))
#define EOC_OPCODE_HOLD		0x01
#define EOC_OPCODE_RTN		0xf0
#define EOC_OPCODE_SLFTST	0x02
#define EOC_OPCODE_REQCOR	0x07
#define EOC_OPCODE_REQEND	0x08
#define EOC_OPCODE_NOTCOR	0x0b
#define EOC_OPCODE_NOTEND	0x0d
#define EOC_OPCODE_EOD		0x0e
#define EOC_OPCODE_REQTPU	0x13
#define EOC_OPCODE_GNTPDN	0x16
#define EOC_OPCODE_REJPDN	0x83
#define EOC_OPCODE_NEXT		0x10
#define EOC_OPCODE_WRITE_0	0x20
#define EOC_OPCODE_WRITE_1	0x23
#define EOC_OPCODE_WRITE_2	0x25
#define EOC_OPCODE_WRITE_3	0x26
#define EOC_OPCODE_WRITE_4	0x29
#define EOC_OPCODE_WRITE_5	0x2A
#define EOC_OPCODE_WRITE_6	0x2C
#define EOC_OPCODE_WRITE_7	0x2F
#define EOC_OPCODE_WRITE_8	0x31
#define EOC_OPCODE_WRITE_9	0x32
#define EOC_OPCODE_WRITE_A	0x34
#define EOC_OPCODE_WRITE_B	0x37
#define EOC_OPCODE_WRITE_C	0x38
#define EOC_OPCODE_WRITE_D	0x3B
#define EOC_OPCODE_WRITE_E	0x3D
#define EOC_OPCODE_WRITE_F	0x3E
#define EOC_OPCODE_READ_0	0x40
#define EOC_OPCODE_READ_1	0x43
#define EOC_OPCODE_READ_2	0x45
#define EOC_OPCODE_READ_3	0x46
#define EOC_OPCODE_READ_4	0x49
#define EOC_OPCODE_READ_5	0x4A
#define EOC_OPCODE_READ_6	0x4C
#define EOC_OPCODE_READ_7	0x4F
#define EOC_OPCODE_READ_8	0x51
#define EOC_OPCODE_READ_9	0x52
#define EOC_OPCODE_READ_A	0x54
#define EOC_OPCODE_READ_B	0x57
#define EOC_OPCODE_READ_C	0x58
#define EOC_OPCODE_READ_D	0x5B
#define EOC_OPCODE_READ_E	0x5D
#define EOC_OPCODE_READ_F	0x5E
#define EOC_OPCODE_NEXT		0x10
#define EOC_OPCODE_REQPDN	0x15
#define EOC_OPCODE_DGASP	0xE7
#define EOC_OPCODE_UTC		0x04

#define EOC_PARITY(x)		(x & 0x0008)
#define EOC_PARITY_ODD		0x0008
#define EOC_PARITY_EVEN		0x0000
#define EOC_ENCODED_PARITY_ODD		0x2000
#define EOC_ENCODED_PARITY_EVEN		0x0000



typedef enum { _idle =0, _utc, _exe, _preread, _read, _prewrite, _write} eoc_state;

struct eoc_registers {
	char vendorID[8];	/* reg 0 */
	char revision[2];	/* reg 1 */
	char serial[32]; 	/* reg 2 */
	char selftest[1];	/* reg 3 */
	char vendor1[1];	/* reg 4 */
	char vendor2[1];	/* reg 5 */
	char attenuation[1];/* reg 6 */
	char SNRmargin[1];	/* reg 7 */
	char ATURconfig[30];/* reg 8 */
						/* no reg 9 reserved for future */
	char linkstate;		/* reg A */
						/* no reg B to F reserved for future */
};


void eoc_init(void); 
u_int16_t eoc_decode(unsigned char b1, unsigned char b2);
void eoc_execute(u_int16_t eocmesval);
int eoc_read_next();
void eoc_write_data(u_int16_t code);
int parse_eoc_buffer(unsigned char *buffer, int bufflen);
void get_oec_answer(unsigned char *eocoutbuff);
int has_eocs(void);
void eoc_encode(u_int16_t eoc_opcode);
