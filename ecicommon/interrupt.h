/*
 *	Interupt.c
 * 
 *	Author : Valette Jean-Sébastien
 *
 * Creation : 30 09 2004
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
*/

#ifdef _KERNEL_
#include <linux/string.h>
#else
#include <string.h>
#endif
#include <linux/errno.h>

#define INT_MAXPIPE_SIZE 64
#define INT_BUFFER_LEN (INT_MAXPIPE_SIZE *2)

int parse_interrupt_buffer(unsigned char *buffer,  int buffer_size,
								unsigned char *resp, int *resp_size);
