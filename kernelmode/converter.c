/*
  

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************

 	Convert .bin file of usermode to usb_packet.h compatible format
	Output on stdout
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>


/*
	From ECI-LOAD2
*/

int convert(FILE *fp)
{
	unsigned char b[8];
	unsigned char *buf=0;
	int size;
	int r,i;

	while((r=fread(b,1,sizeof(b),fp)) == sizeof(b))
	{
	
		/*p->request_type = b[0];
			p->request      = b[1];
		p->value        = (b[2]<<8) | b[3];
		p->index        = (b[4]<<8) | b[5];
		p->size         = (b[6]<<8) | b[7];*/
		printf("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
			b[0], b[1], b[3], b[2], b[5], b[4], b[7], b[6]);

		if ((size = (b[6]<<8) | b[7]))
		{
			buf = (unsigned char *)realloc(buf,size);
			if (!buf)
			{
				printf(" can't allocate %d bytes\n", size);
				return 0;
			}
	
			if (!(b[0] & 0x80))
			{
				if ((r=fread(buf,1,size,fp)) != size)
				{
					printf(" read %d bytes instead of %d\n",
						   r,size);
					return 0;
				}
				for(i = 0; i<r; i++)
				{
					if(!(i%8)) printf("\n");				
					printf ("0x%02x, ",buf[i]);
				}
				printf("\n");
			}
		}
	}
	return -1;
}
int main(int argc, char *argv[])
{
	const char * file = argv[1];
	FILE *f;

	f=fopen("eci_wan3.bin","rb");

	if (!convert(f))
	{
		printf("Conversion failed!\n");
		return -1;
	}

	printf("Conversion Success\n");


	return 0;
}
