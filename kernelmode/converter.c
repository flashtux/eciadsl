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
#include <time.h>

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
	
		/*
      p->request_type = b[0];
			p->request      = b[1];
      p->value        = (b[2]<<8) | b[3];
      p->index        = (b[4]<<8) | b[5];
      p->size         = (b[6]<<8) | b[7];
    */

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

        printf("      ");
				for(i = 0; i<r; i++)
				{
					printf ("0x%02x, ",buf[i]);
          if ((i%8) == 7) printf("\n      ");
				}
				printf("\n");
			}
		}
	}
	return -1;
}

void usage()
{
  printf("usage: converter file.bin\n");
  printf("output a usb_packets.h\n");
  exit (-1);
}

int main(int argc, char *argv[])
{
  const char * file = NULL;
	FILE * fp;
  int i;
  time_t t;

  for (i=1;i<argc;i++)
    {
      if (file == NULL)
        file = argv[i];
      else
        usage();
    }

  if (file == NULL)
    usage();

	fp=fopen(file,"rb");

  time(&t);

  printf(
    "#ifndef USB_PACKETS_H\n"
    "#define USB_PACKETS_H\n"
    "\n"
    "/* Generated on %.24s from :\n"
    "   %s\n"
    "*/\n"
    "\n"
    "static const unsigned char eci_init_setup_packets[] = {\n",
    ctime(&t),file);

	if (!convert(fp))
    {
      printf("Conversion failed!\n");
      return -1;
    }

  printf(
    "0x00\n"
    "};\n"
    "\n"
    "#endif\n");

	return 0;
}
