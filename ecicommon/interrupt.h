/*
 *	Interupt.h
 * 
 *	Author : Valette Jean-S�bastien
 *
 * Creation : 30 09 2004
 * 
 *
 * ****************************************************************************
 * File		: 	$RCSfile$
 * Version	:	$Revision$
 * Modified by	:	$Author$ ($Date$)
 * Licence	:	GPL
 * ****************************************************************************
 * 
 * Include for the dsp int buffer handling.
 * 
*/

#ifdef _KERNEL_
#include <linux/string.h>
#else
#include <string.h>
#endif
#include <linux/errno.h>
#include "dsp.h"

#define INT_MAXPIPE_SIZE 64
#define INT_BUFFER_LEN (INT_MAXPIPE_SIZE *2)
#define INT_MIN_BUFFER_SIZE 8

union ep_int_buf {
	unsigned char raw[INT_BUFFER_LEN];
	struct {
		unsigned char status;	/*	0xf0	-- 0*/
		unsigned char frameid;	/* uised to indicate which frame is sent
									 go one by one step, some are unused -- 1*/
		unsigned char unknown1[2];	/* 2 unknown bytes  -- 2 -- 3*/
		unsigned char eoc[36];		/* 36 byte eoc buffer -- 4 -- 39*/
		unsigned char unknown2[88];	/* 2 unknown bytes  -- 40 -- 128*/
	}buffer;
};


int parse_interrupt_buffer(unsigned char *buffer,  int buffer_size,
								unsigned char *resp, int *resp_size,
								struct gs707x_dsp *dsp);
