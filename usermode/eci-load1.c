/*
  Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 17/07/2001
  Licence : GPL

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

  31/07/2002, Benoit PAPILLAULT
  Added a verbose mode. By default, few lines are output. This removes
  a bug when eci-load1 is launch by initlog (like in boot script).
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "pusb.h"
#include "modem.h"

#define TIMEOUT 1000
#define ECILOAD_TIMEOUT 20

/* just for testing without USB */
/*#define TESTECI*/

struct eci_firm_block
{
	unsigned short addr;
	unsigned short len;
	unsigned char * content;
};

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

pid_t mychild_pid=0;


/*
  Load a firmware block. content should be either NULL
  or the value returned from a previous call to eci_firm_block_read()
  Return 1 is sucessfull.
*/

int eci_firm_block_read(FILE *fp, struct eci_firm_block *p)
{
	unsigned char b[4];
	int r;

	if ((r = fread(b, 1, sizeof(b), fp)) != sizeof(b))
	{
		printf("read %d bytes instead of %d\n", r, sizeof(b));
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

	if ((r = fread(p->content, 1, p->len, fp)) != p->len)
	{
		printf("read %d bytes instead of %d\n", r, p->len);
		return 0;
	}

	return 1;
}

int eci_load1(const char * file, unsigned short vid1, unsigned short pid1,
			  int option_verbose)
{
	FILE * fp ;
	struct eci_firm_block block;
	long size ;
	pusb_device_t dev;
	int n = 0;

	/* open the file */

	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		perror(file);
		return 0;
	}

	/* compute the file size */

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* open the USB device */
#ifndef TESTECI
	dev = pusb_search_open(vid1, pid1);
	if (dev == NULL)
	{
		printf("can't find your " EZUSB_NAME "\n");
		fclose(fp);
		return 0;
	}

	/* initialize the USB device */

	if (pusb_set_configuration(dev, 1) < 0)
	{
		printf("can't set configuration 1\n");
		pusb_close(dev);
		fclose(fp);
		return 0;
	}

	if (pusb_set_interface(dev, 0, 1) < 0)
	{
		perror("can't set interface 0 to use alt setting 1\n");
		pusb_close(dev);
		fclose(fp);
		return 0;
	}
#endif
	block.content = NULL;

	while (ftell(fp) < size)
	{
		n++;

		if (!eci_firm_block_read(fp, &block))
		{
			printf("can't read block\n");
#ifndef TESTECI
			pusb_close(dev);
#endif
			fclose(fp);
			return 0;
		}

		if (option_verbose)
			printf("block %3d: addr 0x%04x - length %2u: ",
				   n, block.addr, block.len);

#ifndef TESTECI
		if (pusb_control_msg(dev, 0x40, 0xa0, block.addr, 0,
							 block.content, block.len, TIMEOUT) < 0)
		{
			if (option_verbose)
				printf("failed\n");

			pusb_close(dev);
			fclose(fp);
			return 0;
		}
#endif

		if (option_verbose)
			printf("OK\n");
	}
#ifndef TESTECI
	pusb_close(dev);
#endif
	fclose(fp);

	return 1;
}

int check_modem(unsigned short vid2, unsigned short pid2)
{
	pusb_device_t dev;
	struct timeval tv, start, now;
	int i, r;

#define RETRY_PERIOD 10 /* time between retries, in ms */
#define RETRY_MAX    1000

	gettimeofday(&start, NULL);

	/* let the new USB device the time to initialize,
	   or else, it seems to crash the USB device itself :-(
	   or maybe only the Linux USB stack ... 
	*/

	sleep(1);

	for (i=0; i<RETRY_MAX; i++)
	{
		fd_set rset;
		FD_ZERO(&rset);

		dev = pusb_search_open(vid2, pid2);
		if (dev != NULL)
			break;

		tv.tv_sec = RETRY_PERIOD / 1000;
		tv.tv_usec = (RETRY_PERIOD % 1000) * 1000;

		r = select(1, NULL, NULL, NULL, &tv);
		if (r < 0)
		{
			perror("select");
			break;
		}
	}

	if (dev == NULL)
	{
		printf("can't find your " GS_NAME "\n");
		return 0;
	}

	gettimeofday(&now, NULL);
	
	printf(
		GS_NAME " modem found (in %ld ms): GREAT!\n",
		(long) (( (now.tv_sec - start.tv_sec) * 1000) 
				+ ((now.tv_usec - start.tv_usec) / 1000) ));
	pusb_close(dev);
	return 1;
}

void version(const int full)
{
	printf(PRODUCT_NAME " (" PRODUCT_ID ") " PRODUCT_VERSION " (" __DATE__ " " __TIME__ ")\n");
	if (full)
		printf("%s\n", id);
	_exit(0);
}

void usage(const int ret)
{
	printf("usage:\n");
	printf("       eci-load1 [<switch>..] [VID1 PID1 VID2 PID2] firmware.bin\n");
	printf("switches:\n");
	printf("       -v or --verbose   be verbose\n");
	printf("       -h or --help      show this help message then exit\n");
	printf("       -V or --version   show version information then exit\n");
	_exit(ret);
}

void sigtimeout()
{
	printf("ECI load 1: timeout\n");
	fflush(stdout);
	if (mychild_pid)
		kill(mychild_pid, SIGINT);
	_exit(-1);
}

int main(int argc, char *argv[])
{
	const char * file;
	int status;
	unsigned short vid1, vid2;
	unsigned short pid1, pid2;
	int i, j;
	int option_verbose = 0;
	pid_t child_pid;

	/* parse command line options */

	for (i=1, j=1; i<argc; i++)
	{
		if ((strcmp(argv[i], "-V") == 0) || (strcmp(argv[i], "--version") == 0))
			version(0);
		else
		if (strcmp(argv[i], "--full-version") == 0)
			version(1);
		else
		if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
			usage(0);
		else
		if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0))
			option_verbose = 1;
		else
			argv[j++] = argv[i];
	}
	argc = j;

	/* parse remaining command line arguments */
	if (argc == 2)
	{
		file = argv[1];
		vid1 = EZUSB_VENDOR;
		pid1 = EZUSB_PRODUCT;
		vid2 = GS_VENDOR;
		pid2 = GS_PRODUCT;
	}
	else
	if (argc == 6)
	{
		file = argv[5];
		vid1 = strtoul(argv[1], NULL, 0);
		pid1 = strtoul(argv[2], NULL, 0);
		vid2 = strtoul(argv[3], NULL, 0);
		pid2 = strtoul(argv[4], NULL, 0);
	}
	else
		usage(-1);

	signal(SIGALRM, sigtimeout);
	alarm(ECILOAD_TIMEOUT);

	child_pid = fork();
	if (child_pid == -1)
	{
		alarm(0);
		perror("fork failed");
		printf("ECI load 1: failed\n");
		fflush(stdout);
		_exit(-1);
	}
	if (child_pid == 0)
	{
		if (!eci_load1(file, vid1, pid1, option_verbose) || !check_modem(vid2, pid2))
		{
			alarm(0);
			printf("ECI load 1: failed\n");
			fflush(stdout);
			_exit(-1);
		}
		_exit(0);
	}
	mychild_pid = child_pid;
	/* wait for child process and intercept child abortion due to signal (user abort or timeout) */
	do
		if (waitpid(child_pid, &status, 0) == -1)
			if (errno == EINTR)
			{
				perror("child failed (aborted)");
				_exit(1);
			}
	while(errno != ECHILD);
	alarm(0);

	if (status)
		return 1;

	printf("ECI load 1: success\n");
	fflush(stdout);
		
	return 0;
}
