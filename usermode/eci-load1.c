/*
  Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 17/07/2001

  First : Upload the firwmare to the EZUSB part of the ECI USB ADSL modem
  It uses a firmware file generate by a perl script (eci_uc.pl) from Windows
  USB log. The generated file name is eci_firmware.bin.

  23/11/2001 : Added some delay before testing for the existence of
  the WAN USB interface (ECI_xxx).

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************

  Tested with :
  eci_firm_1.bin                     : file is corrupted or in wrong format
  eci_firm_bourrin.bin               : works
  eci_firm_Mr-Ragga.Mr-ragga.log     : works
  eci_firm_kit_wanadoo.bin           : works
  eci_firm_Mr-Ragga.Mr-ragga.log.1   : does not work.

  Resulting USB devices :

  *  eci_firm_bourrin.bin

Device: VendorID=0x0915 ProductID=0x8000 Class=ff/ff/ff, 1 configuration(s)
Manufacturer: GlobeSpan Inc. Product: USB-ADSL Modem SN: FFFFFF
  configuration 1, 1 interface(s) []
    interface 0 alt 0 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x82 [Bulk] 64 bytes 0 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 1 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 1008 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 2 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 912 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 3 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 736 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 4 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 448 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 5 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 240 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 6 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 96 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms

  * eci_firm_Mr-Ragga.Mr-ragga.log

Device: VendorID=0x0915 ProductID=0x8000 Class=ff/ff/ff, 1 configuration(s)
Manufacturer: GlobeSpan Inc. Product: USB-ADSL Modem SN: FFFFFF
  configuration 1, 1 interface(s) []
    interface 0 alt 0 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x82 [Bulk] 64 bytes 0 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 1 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 1023 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 2 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 912 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 3 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 736 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 4 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 448 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 5 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 240 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 6 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 96 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms

  * eci_firm_kit_wanadoo.bin
 
Device: VendorID=0x0915 ProductID=0x8000 Class=ff/ff/ff, 1 configuration(s)
Manufacturer: GlobeSpan Inc. Product: USB-ADSL Modem SN: FFFFFF
  configuration 1, 1 interface(s) []
    interface 0 alt 0 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x82 [Bulk] 64 bytes 0 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 1 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 1008 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 2 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 912 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 3 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 736 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 4 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 448 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 5 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 240 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
    interface 0 alt 6 class ff/ff/ff, 3 endpoint(s) []
      endpoint 0x88 [Isoc] 96 bytes 1 ms
      endpoint 0x02 [Bulk] 64 bytes 0 ms
      endpoint 0x86 [Intr] 64 bytes 3 ms
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "pusb.h"

/* Vendor/ProdID for "ECI Telecom USB ADSL Loader" */
#define EZUSB_NAME    "ECI Telecom USB ADSL Loader"
#define EZUSB_VENDOR  0x0547
#define EZUSB_PRODUCT 0x2131

/* Vendor/ProdID for "ECI Telecom USB ADSL WAN Modem" */
#define ECI_NAME      "ECI Telecom USB ADSL WAN Modem"
#define ECI_VENDOR    0x0915
#define ECI_PRODUCT   0x8000

#define TIMEOUT 1000

/* just for testing without USB */
/*#define TESTECI*/

struct eci_firm_block
{
	unsigned short addr;
	unsigned short len;
	unsigned char * content;
};

/*
  Load a firmware block. content should be either NULL
  or the value returned from a previous call to eci_firm_block_read()
  Return 1 is sucessfull.
*/

int eci_firm_block_read(FILE *fp, struct eci_firm_block *p)
{
	unsigned char b[4];
	int r;

	if ((r=fread(b,1,sizeof(b),fp)) != sizeof(b))
	{
		printf("read %d bytes instead of %d\n",r,sizeof(b));
		return 0;
	}

	p->addr = (b[0] << 8) | b[1];
	p->len  = (b[2] << 8) | b[3];

	p->content = (unsigned char *) realloc (p->content, p->len);
	if (p->content == NULL)
	{
		perror("malloc");
		return 0;
	}

	if ((r=fread(p->content,1,p->len,fp)) != p->len)
	{
			printf("read %d bytes instead of %d\n",r,p->len);
			return 0;
	}

	return 1;
}

int eci_load1(const char * file)
{
	FILE * fp ;
	struct eci_firm_block block;
	long size ;
	pusb_device_t dev;
	int n = 0;

	/* open the file */

	fp = fopen(file,"rb");
	if (fp == NULL)
	{
		perror(file);
		return 0;
	}

	/* compute the file size */

	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	/* open the USB device */
#ifndef TESTECI
	dev = pusb_search_open(EZUSB_VENDOR,EZUSB_PRODUCT);
	if (dev == NULL)
	{
		printf("Can't find your " EZUSB_NAME "\n");
		fclose (fp);
		return 0;
	}

	/* initialize the USB device */

	if (pusb_set_configuration(dev,1) < 0)
	{
		printf("Can't set configuration 1\n");
		pusb_close (dev);
		fclose (fp);
		return 0;
	}

	if (pusb_set_interface(dev,0,1) < 0)
	{
		printf("Can't set interface 0 to use alt setting 1\n");
		pusb_close (dev);
		fclose (fp);
		return 0;
	}
#endif
	block.content = NULL;

	while (ftell(fp) < size)
	{
		n++;

		if (!eci_firm_block_read(fp,&block))
		{
			printf("can't read block\n");
#ifndef TESTECI
			pusb_close (dev);
#endif
			fclose (fp);
			return 0;
		}

		printf("Block %3d : Addr 0x%04x - Length %2u : ",
			   n,block.addr, block.len);
#ifndef TESTECI
		if (pusb_control_msg(dev,0x40,0xa0,block.addr,0,
							 block.content,block.len,TIMEOUT) < 0)
		{
			printf("Failed\n");
			pusb_close (dev);
			fclose (fp);
			return 0;
		}
#endif
		printf("Ok\n");
	}
#ifndef TESTECI
	pusb_close(dev);
#endif
	fclose (fp);

	return 1;
}

void check_modem()
{
	pusb_device_t dev;
	struct timeval tv, start, now;
	int i, r;

#define RETRY_PERIOD 10 /* time between retries, in ms */
#define RETRY_MAX    1000

	gettimeofday(&start,NULL);

	/* let the new USB device the time to initialize,
	   or else, it seems to crash the USB device itself :-(
	   or maybe only the Linux USB stack ... 
	*/

	sleep(1);

	for (i=0;i<RETRY_MAX;i++)
	{
		fd_set rset;
		FD_ZERO(&rset);

		dev = pusb_search_open(ECI_VENDOR,ECI_PRODUCT);
		if (dev != NULL)
			break;

		tv.tv_sec = RETRY_PERIOD / 1000;
		tv.tv_usec = (RETRY_PERIOD % 1000) * 1000;

		r = select(1,NULL,NULL,NULL,&tv);
		if (r < 0)
		{
			perror("select");
			break;
		}
	}

	if (dev == NULL)
	{
		printf("Can't find your " ECI_NAME "\n");
		return ;
	}

	gettimeofday(&now,NULL);
	
	printf(
		"I found your " ECI_NAME " (in less than %ld ms) : GREAT!\n",
		(long) (
			((now.tv_sec - start.tv_sec) * 1000) 
			+ ((now.tv_usec - start.tv_usec) / 1000)
		)
	);
	pusb_close (dev);
}

void usage()
{
	printf("usage: eci-load1 eci_firmware.bin\n");
	exit (-1);
}

int main(int argc, char *argv[])
{
	const char * file = argv[1];

	if (argc != 2)
		usage();

	if (!eci_load1(file))
	{
		printf("ECI Load 1 : failed!\n");
		return -1;
	}

	printf("ECI Load 1 : Success\n");

	check_modem();
	return 0;
}
