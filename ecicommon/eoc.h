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


enum eoc_state { idle =0, utc, preread, read, prewirte, write);

