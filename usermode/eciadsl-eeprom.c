/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 14/02/2002
  Licence: GPL

  THIS PROGRAM WILL MAKE YOUR MODEM UNUSABLE. I REPEAT:
  DO NOT USE THIS PROGRAM.

  Interesting URBs in the Windows USB log file:

URB 11417: out request_type=0x40  request=00df value=0000 index=0000
ff ff ff ff ff ff ff 06 05 04 03 02 01          .............
URB 13062: in  request_type=0xc0  request=00de value=0000 index=0000
ff ff ff ff ff ff ff 06 05 04 03 02 01          .............

  This program change the EZ-USB eeprom and thus its Vendor/Product ID.
  Eeprom is 16 bytes long. Writing time is n+3 seconds where n is the number
  of bytes to write. The green leds switch off during the writing if it was
  blinking.

  To compile: make eciadsl-eeprom
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pusb.h"
#include "modem.h"

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

#define TIMEOUT 20000

void usage()
{
	printf("Usage: eciadsl-eeprom oldVID oldPID newVID newPID\n");
	printf("where: VID = Vendor ID and PID = Product ID\n");
	printf("specify newVID=0 & newPID=0 to revert to the original values (VID/PID=0547/2131)\n");
	exit (-1);
}

int main(int argc, const char *argv[])
{
	pusb_device_t dev;
	int i,r;
	char c;

	/*
	  magic_b should be 0xb0 to use the new vid/pid
	  and 0xff to revert back to the original values
	*/

	unsigned char magic_b      =   0xb0;
	unsigned short device_id  = 0x9999;
	unsigned char mac[6] = { 1, 2, 3, 4, 5, 6 };

	unsigned short old_vid, new_vid;
	unsigned short old_pid, new_pid;

    /* We use 13 bytes length packet, since the Windows driver is
       using 13 bytes packets. If we use 16 bytes packets,
       pusb_control_msg() returns a timeout */

	unsigned char pkt[13];
	unsigned char opkt[13];
	time_t debut, fin;

	if (argc != 5)
		usage();

	old_vid = strtoul(argv[1],NULL,0);
	old_pid = strtoul(argv[2],NULL,0);
	new_vid = strtoul(argv[3],NULL,0);
	new_pid = strtoul(argv[4],NULL,0);

	printf("changing VendorID/ProductID=%04x/%04x to %04x/%04x\n",
		   old_vid,old_pid,new_vid,new_pid);

	pkt[0] = magic_b;
	pkt[1] = new_vid & 0xff;
	pkt[2] = new_vid >> 8;
	pkt[3] = new_pid & 0xff;
	pkt[4] = new_pid >> 8;
	pkt[5] = device_id & 0xff;
	pkt[6] = device_id >> 8;

	/* mac addr */
	pkt[ 7] = mac[5];
	pkt[ 8] = mac[4];
	pkt[ 9] = mac[3];
	pkt[10] = mac[2];
	pkt[11] = mac[1];
	pkt[12] = mac[0];

	if (new_vid == 0 && new_pid == 0)
		pkt[0] = 0xff;

	dev = pusb_search_open(old_vid,old_pid);
	if (dev == NULL)
	{
		printf("I cannot find your " GS_NAME "\n");
		return -1;
	}

	printf("writing:");
	for (i=0;i<sizeof(pkt);i++)
		printf(" %02x",pkt[i]);
	printf("\n");

	printf("Do you want to DESTROY your " GS_NAME "? if so, say y\n");
	c = getchar();
	if (c == 'y' || c == 'Y')
	{
		printf("Ok. burning the modem :-)...\n");

		time(&debut);
		if ((r=pusb_control_msg(dev,0x40,0xdf,0,0,pkt,
								sizeof(pkt),TIMEOUT)) != sizeof(pkt))
		{
			printf("Failed r=%d\n",r);
		}
		else
		{
			time(&fin);
			printf(" ...in %d seconds\n",fin-debut);
		}
	}
	else
	{
		printf("Ok. you choose the safe way\n");
	}

	if ((r=pusb_control_msg(dev,0xc0,0xde,0,0,opkt,
							sizeof(opkt),TIMEOUT)) != sizeof(opkt))
	{
		printf("Failed r=%d\n",r);
		return -1;
	}
	
	printf("read   :");
	for (i=0;i<sizeof(opkt);i++)
		printf(" %02x",opkt[i]);
	printf("\n");

	return 0;
}

