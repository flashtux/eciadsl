/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 20/02/2002

  Goals: convert .bin files used by eci-load1 to Intel HeX format,
  as documented on http://www.8052.com/tutintel.phtml
*/

#include <stdio.h>
#include <stdlib.h>

static const char id[] = "@(#) $Id$";

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

/*
  write an Intel Hex Record into fp. Returns 1 if successful
  record_type = 0x00 => normal record
  record_type = 0x01 => end of file
*/

int eci_hex_block_write(FILE *fp, struct eci_firm_block *p,
						unsigned char record_type)
{
	const unsigned char record_marker = ':';
	unsigned int record_length = 0;
	unsigned int record_address = 0;
	unsigned int crc = 0;
	int i;

	if (record_type == 0)
	{
		record_length = p->len;
		record_address = p->addr;
	}

	if (fprintf(fp,"%c%02X%04X%02X",record_marker, record_length,
				record_address, record_type) < 0)
		return 0;

	crc = record_length + (record_address >> 8) + (record_address & 0xff)
		+ record_type;

	for (i=0;i<record_length;i++)
	{
		if (fprintf(fp,"%02X",p->content[i]) < 0)
			return 0;
		crc += p->content[i];
	}

	/* finish computing the crc */
	crc = (0x100 - (crc & 0xff)) & 0xff;
	if (fprintf(fp,"%02X\n",crc) < 0)
		return 0;

	return 1;
}

int bin2hex(const char * infile, const char * outfile)
{
	FILE * infp, * outfp;
	struct eci_firm_block block;
	long size;
	int n = 0;

	/* open the input file for reading */

	infp = fopen(infile,"rb");
	if (infp == NULL)
	{
		perror(infile);
		return -1;
	}

	/* open the output file for writing */

	outfp = fopen(outfile,"wb");
	if (outfp == NULL)
	{
		perror(outfile);
		fclose (infp);
		return -1;
	}

	/* compute the file size */

	fseek(infp,0,SEEK_END);
	size = ftell(infp);
	fseek(infp,0,SEEK_SET);

	block.content = NULL;

	while (ftell(infp) < size)
	{
		n++;

		if (!eci_firm_block_read(infp,&block))
		{
			printf("I cannot read record %d\n", n);
			fclose (outfp);
			fclose (infp);
			return -1;
		}

		printf("Block %3d : Addr 0x%04x - Length %2u : Read",
			   n,block.addr, block.len);

		/* we skip over record whose addr is >= 0x4000 */
		if (block.addr < 0x4000)
		{
			if (!eci_hex_block_write(outfp, &block, 0))
			{
				printf("\nI cannot write record %d\n", n);
				fclose (outfp);
				fclose (infp);
				return -1;
			}

			printf("/Written\n");
		}
		else
		{
			printf("/Skip\n");
		}
	}

	if (!eci_hex_block_write(outfp, &block, 1))
	{
		printf("I cannot write end of file record");
		fclose (outfp);
		fclose (infp);
		return -1;
	}

	fclose (outfp);
	fclose (infp);

	return 0;
}

void usage()
{
	printf("Usage: bin2hex firmware.bin firmware.ihx\n");
	exit (-1);
}

int main(int argc, char *argv[])
{
	const char * infile = NULL;
	const char * outfile = NULL;
	int i;

	for (i=1;i<argc;i++)
	{
		if (infile == NULL)
			infile = argv[i];
		else if (outfile == NULL)
			outfile = argv[i];
		else
			usage();
	}

	if (infile == NULL || outfile == NULL)
		usage();

	return bin2hex(infile,outfile);
}
