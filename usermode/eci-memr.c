/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 14/02/2002

  This program read the whole 8051 memory. It can be run before loading
  the firmware or after, even if pppoeci is running.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pusb.h"

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

#define TIMEOUT 200

void usage()
{
	printf("Usage: eci-memr VID PID\n");
	printf("where: VID = Vendor ID and PID = Product ID\n");
	exit (-1);
}

void
print_char(unsigned char c)
{

	if(c >= ' ' && c < 0x7f)
		printf("%c", c);
	/*
	else if (c=='\r' || c=='\n' || c=='\t' || c=='\b')
	printf("%c",c);
	*/
	else
		printf(".");
}

void
dump(unsigned char *buf, int len)
{
	int i, j;

	for(i = 0; i < len; i += 16) {

		for(j = i; j < len && j < i + 16; j++)
			printf("%02x ", buf[j]);

		for(; j < i + 16; j++)
			printf("   ");

		for(j = i; j < len && j < i + 16; j++)
			print_char(buf[j]);

		printf("\n");
	}
}

int main(int argc, const char *argv[])
{
	pusb_device_t dev;
	int r;
	unsigned int vid, pid;
	unsigned char pkt[16];
	unsigned int addr;

	if (argc != 3)
		usage();

	vid = strtoul(argv[1],NULL,0);
	pid = strtoul(argv[2],NULL,0);

	dev = pusb_search_open(vid,pid);
	if (dev == NULL)
	{
		printf("I cannot find your EZ-USB device\n");
		return -1;
	}

	for (addr=0;addr<0x10000;addr += sizeof(pkt))
	{
		memset(pkt,0,sizeof(pkt));
		
		if ((r=pusb_control_msg(dev,0xc0,0xa0,addr,0,pkt,
								sizeof(pkt),TIMEOUT)) != sizeof(pkt))
		{
			printf("Failed r=%d\n",r);
		}
		
		printf("read %04x : ",addr);
		dump(pkt,sizeof(pkt));
	}

	pusb_close(dev);

	return 0;
}

