/*
  Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 20/07/2001
  Licence : GPL

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************

  Used to initalized the secondary USB device (created by eci-load1).
  This should synchronize the ADSL link (fixed led).

  Use a file generated by : ./eci_vendor_device.pl < adsl-wan-modem-3.log

  23/11/2001 : Major changes. We use the Wanadoo kit to generate the log.
  On a first time, we will only read/write vendor device.

  31/07/2002, Benoit PAPILLAULT
  Added a verbose mode. By default, few lines are output. This removes
  a bug when eci-load2 is launch by initlog (like in boot script).
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "pusb.h"
#include "modem.h"

#define TIMEOUT 2000
#define INFINITE_TIMEOUT 24*60*60*1000 /* 24 hours should be enought */
#define ECILOAD_TIMEOUT 60

/* just for testing without USB */
/*#define TESTECI*/

struct usb_block
{
	unsigned char   request_type;
	unsigned char   request;
	unsigned short  value;
	unsigned short  index;
	unsigned short  size;
	unsigned char* buf; /* buf's content is stored in the .bin file
							only for OUT request (ie request_type & 0x80)=0 */
};

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

int usb_block_read(FILE* fp, struct usb_block* p)
{
	unsigned char b[8];
	int r;

	if ((r = fread(b, sizeof(char), sizeof(b), fp)) != sizeof(char)*sizeof(b))
	{
		printf("usb_block_read: read %d bytes instead of %d\n", r, sizeof(char)*sizeof(b));
		return 0;
	}

	p->request_type = b[0];
	p->request      = b[1];
	p->value        = (b[2]<<8) | b[3];
	p->index        = (b[4]<<8) | b[5];
	p->size         = (b[6]<<8) | b[7];

	if (p->size != 0)
	{
		p->buf = (unsigned char*)realloc(p->buf, p->size);
		if (p->buf == NULL)
		{
			printf("usb_block_read: can't allocate %d bytes\n",
				   p->size);
			return 0;
		}

		if ((p->request_type & 0x80) == 0)
			if ((r=fread(p->buf, sizeof(char), p->size, fp)) != p->size)
			{
				printf("usb_block_read: read %d bytes instead of %d\n",
					   r, p->size);
				return 0;
			}
	}

	return 1;
}

void print_char(unsigned char c)
{

	if (c>=' ' && c<0x7f)
		printf("%c", c);
	/*
	else if (c=='\r' || c=='\n' || c=='\t' || c=='\b')
	printf("%c", c);
	*/
	else
		printf(".");
}

void dump(unsigned char* buf, int len)
{
  int i, j;
  
	for (i = 0; i < len; i+=16)
	{
		for (j = i; j < len && j < i + 16; j++)
			printf("%02x ", buf[j]);
		for (; j < i + 16; j++)
			printf("   ");
		for (j = i; j < len && j < i + 16; j++)
			print_char(buf[j]);
		printf("\n");
	}
	
}

pid_t mychild_pid=0;

void read_endpoint(pusb_device_t dev, int epnum, int option_verbose)
{
	pid_t child_pid;
	pusb_endpoint_t ep;
	unsigned char lbuf[0x40];
	int ret, i;
	/* indicates if we already received 16 bytes packets */
	int pkt16_received = 0;

	fflush(stdout);
	fflush(stderr);

	child_pid = fork();
	if (child_pid)
	{
		if (mychild_pid)
		{
			perror("damn");
			_exit(-1);
		}
		mychild_pid = child_pid;
		return;
	}

	/* here, we are running in a child process */
	ep = pusb_endpoint_open(dev, epnum, O_RDONLY);
	if (ep == NULL)
	{
		perror("pusb_endpoint_open");
		_exit(-1);
	}

	/* we DO NOT stop after receiving 100 interrupts */
	for (i = 0; /* i < 100 */; i++)
	{

		/*
		  24/11/2001
		  Warning : we use a TIMEOUT. This is very important and
		  this is caused by the current implementation of the pusb library.
		  Without a timeout, we can have several process trying to reap URB at
		  once. And what can happen and what happens really, is that one
		  process reaps the URB from the other process. As as result, it mixes
		  data from various endpoint

		  Solution : we use a big timeout. We assume that 24 hours should be 
		  enought.

		  25/11/2001
		  Warning again : if we use a TIMEOUT, pusb implementation on Linux
		  use USBDEVFS_BULK, which locks down a big semaphore and then wait.
		  Thus preventing any other ioctl() and URB to be submitted on the
		  device. So we revert to the old behaviour : NO TIMEOUTS ...
		*/

		ret = pusb_endpoint_read(ep, lbuf, sizeof(lbuf), 0);
		
		if (ret < 0)
		{
			printf("error reading interrupts\n");
			break;
		}
		
		if (option_verbose)
		{
			printf("endpoint %02x, packet len = %d\n", epnum, ret);
			dump(lbuf, ret);
			
			fflush(stdout);
			fflush(stderr);
		}

		/* try to stop the sleep() function in our father, seems to work */
		if (epnum == 0x86 && ret != 64)
			kill(getppid(), SIGUSR1);

		if (ret == 16)
			pkt16_received = 1;

		/* we consider that a 64 bytes packet means that the line is
		   synchronized. So we stop. We wait first to have received
		   a 16 bytes packets, in order to be sure that this 64 bytes
		   packet is not an unread packet from a previous instance of this
		   program (or another)
		*/

		if (pkt16_received && ret == 64)
			break;
	}

	pusb_endpoint_close(ep);
	
	_exit(0);
}

int eci_load2(const char* file, unsigned short vid2, unsigned short pid2,
			  int option_verbose)
{
	FILE* fp ;
	struct usb_block block;
	long size ;
	pusb_device_t dev;
	int n = 0, r, i;

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
	dev = pusb_search_open(vid2, pid2);
	if (dev == NULL)
	{
		printf("can't find your " GS_NAME "\n");
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

	/* let some time... is it needed ? */
	sleep(1);

	if (pusb_claim_interface(dev, 0) < 0)
	{
		printf("can't claim interface 0\n");
		pusb_close(dev);
		fclose(fp);
		return 0;
	}

	/* warning : orginal setting is 0,4 (source : Windows driver) */

	if (pusb_set_interface(dev, 0, 4) < 0)
	{
		printf("can't set interface 0 to use alt setting 4\n");
		pusb_close(dev);
		fclose(fp);
		return 0;
	}

	read_endpoint(dev, 0x86, option_verbose);
	/*read_endpoint(dev, 0x88);*/

#endif

	memset(&block, 0, sizeof(block));

	while (ftell(fp) < size)
	{
		n++;

		if (!usb_block_read(fp, &block))
		{
			printf("can't read block\n");
#ifndef TESTECI
			pusb_close(dev);
#endif
			fclose(fp);
			return 0;
		}

		if (option_verbose)
			printf ("Block %3d: request_type=%02x request=%02x value=%04x index=%04x size=%04x : ",
					n, block.request_type, block.request, block.value,block.index, block.size);

#ifndef TESTECI

		if ((r=pusb_control_msg(dev, block.request_type, block.request,
							 block.value, block.index, block.buf,
							 block.size, TIMEOUT)) != block.size)
		{
			if (option_verbose)
				printf("failed r=%d\n", r);

			pusb_close(dev);
			fclose(fp);
			return 0;
		}

		if (r != block.size)
			printf("(expected %d, got %d) ", block.size, r);
#else
		r = block.size;
#endif

		if (option_verbose)
		{
			printf("OK\n");

			if ((block.request_type & 0x80))
			{
				for (i = 0; i < r; i++)
					printf("%02x ", block.buf[i]);
				printf("\n");
			}
		}

#ifndef TESTECI
		if (block.request_type == 0x40 && block.request == 0xdc
			&& block.value == 0x00 && block.index == 0x00)
		{
			struct timeval tv1, tv2;

			if (option_verbose)
			{
				printf("waiting.. ");
				
				fflush(stdout);
				fflush(stderr);

				gettimeofday(&tv1, NULL);
			}

			/*
			  25/11/2001
			  This call to pause() should be unblock by our son sending
			  SIGUSR1
			*/

			pause();

			if (option_verbose)
			{
				gettimeofday(&tv2, NULL);
				printf("waited for %ld ms\n",
					   (long)(tv2.tv_sec - tv1.tv_sec) * 1000
					   + (long)(tv2.tv_usec - tv1.tv_usec) / 1000);
			}
		}

		fflush(stdout);
		fflush(stderr);
#endif

	}
#ifndef TESTECI

	/* we try to send a small packet */
/*
	{
		unsigned char buf_data[] = {
			0x00, 0x80, 0x02, 0x30, 0x00, 0xc0, 0x21, 0x01, 0x00, 0x00, 0x28, 0x05, 0x06, 0x2e, 0xac, 0x7d, 
			0x04, 0x0d, 0x03, 0x06, 0x11, 0x04, 0x06, 0x4e, 0x13, 0x17, 0x01, 0x57, 0x44, 0x2f, 0x7c, 0xd1, 
			0x11, 0x48, 0x07, 0x9b, 0x53, 0x9e, 0xaf, 0xe6, 0xf8, 0x98, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
			0x00, 0x80, 0x02, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x2a, 0x6a, 0xf8, 0xbc, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
		};

		pusb_endpoint_t ep_data;

		ep_data = pusb_endpoint_open(dev, 0x2, O_RDONLY);
		if (ep_data == NULL)
		{
			perror("Can't open endpoint 2");
			pusb_close(dev);
			return 0;
		}

		r = pusb_endpoint_write(ep_data, buf_data, sizeof(buf_data), TIMEOUT);
		if (r != sizeof(buf_data))
		{
			perror("pusb_endpoint_write");
			pusb_endpoint_close(ep_data);
			pusb_close(dev);
			return 0;
		}

		pusb_endpoint_close (ep_data);
	}
*/
	/*
	  25/11/2001 . Benoit PAPILLAULT.
	  We never release the interface, since it is still in use by our
	  son. If we do, we can the following message in syslog (with our
	  son's pid): "usbdevfs: process 1133 (eci-load2) did not claim
	  interface 0 before use"
	*/

	/*pusb_release_interface(dev, 0);*/
	pusb_close(dev);
#endif

	if (ftell(fp) != size)
		printf("file size = %ld <=> bytes read = %ld\n", size, ftell(fp));

	fclose(fp);

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
	printf("       eci-load2 [<switch>..] [VID2 PID2] <synch.bin>\n");
	printf("switches:\n");
	printf("       -v or --verbose   be verbose\n");
	printf("       -h or --help      show this help message then exit\n");
	printf("       -V or --version   show version information then exit\n");
	_exit(ret);
}

void sigusr1()
{
	/*printf("get SIGUSR1\n");*/
}

void sigtimeout()
{
	printf("ECI load 2: timeout\n");
	fflush(stdout);
	if (mychild_pid)
		kill(mychild_pid, SIGINT);
	exit(-1);
}


int main(int argc, char** argv)
{
	const char* file;
	int status;
	unsigned short vid2, pid2;
	int i, j;
	int option_verbose = 0;

	for (i = 1, j = 1; i < argc; i++)
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
	
	if (argc != 4 && argc != 2)
		usage(-1);

	if (argc == 2)
	{
		file = argv[1];
		vid2 = GS_VENDOR;
		pid2 = GS_PRODUCT;
	}
	else
	{
		file = argv[3];
		vid2 = strtoul(argv[1], NULL, 0);
		pid2 = strtoul(argv[2], NULL, 0);
	}

	signal(SIGUSR1, sigusr1);
	signal(SIGALRM, sigtimeout);
	alarm(ECILOAD_TIMEOUT);
	if (!eci_load2(file, vid2, pid2, option_verbose))
	{
		printf("ECI load 2: failed\n");
		fflush(stdout);
	}

	/* wait for child process and intercept child abortion due to signal (user abort or timeout) */
	do
		if (waitpid(mychild_pid, &status, 0) == -1)
			if (errno == EINTR)
			{
				perror("child failed (aborted)");
				_exit(1);
			}
	while(errno != ECHILD);
	alarm(0);

	if (status)
		return 1;

	printf("ECI load 2: success\n");
	fflush(stdout);

	return 0;
}
