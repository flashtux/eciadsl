/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 21/02/2002

  Goals: read/write from/to EZUSB memory (external memory whose first bytes
  are also code memory) to/from an Intel HeX record
*/

#include <stdio.h>
#include <stdlib.h>
#include "pusb.h"

static const char id[] = "@(#) $Id: ";

#define TIMEOUT 200

enum {
	TYPE_NORMAL = 0,
	TYPE_EOF    = 1
};

struct ihx
{
	unsigned char  len;
	unsigned short addr;
	unsigned char  type; /* 0 => normal record, 1 => end of file */
	unsigned char * content;
};

const struct ihx ihx_reset_in = {
	1, 0x7f92, TYPE_NORMAL, "\x01"
};

const struct ihx ihx_reset_out = {
	1, 0x7f92, TYPE_NORMAL, "\x00"
};

int ihx_init(struct ihx * r)
{
	r->len = 0;
	r->addr = 0;
	r->type = TYPE_NORMAL;
	r->content = NULL;
	return 0;
}

void ihx_done(struct ihx * r)
{
	if (r->len != 0 && r->content != NULL)
		free (r->content);
}

unsigned ihx_compute_crc(const struct ihx * r)
{
	unsigned int crc;
	int i;

	crc = (unsigned int)r->len + (r->addr >> 8) + (r->addr & 0xff) + r->type;
	for (i=0;i<r->len;i++)
		crc += r->content[i];

	crc = (0x100 - (crc & 0xff)) & 0xff;
	return crc;
}

/* return NULL if error */

char * get_byte(char * line, unsigned int * v)
{
	char c;
	int i;

	*v = 0;

	for (i=0;i<2;i++)
	{
		c = * line ++;
		if (c >= '0' && c <= '9')
			*v += c - '0';
		else if (c >= 'a' && c<='f')
			*v += c - 'a' + 10;
		else if (c >= 'A' && c<= 'F')
			*v += c - 'A' + 10;
		else
			return NULL;

		if (i==0)
			*v <<= 4;
	}

	return line;
}

/* return NULL if error */

char * get_word(char * line, unsigned int * v)
{
	int i;
	unsigned int r[2];

	for (i=0;i<2;i++)
	{
		line = get_byte(line,r+i);
		if (line == NULL)
			return NULL;
	}

	*v = (r[0] << 8) | r[1];
	return line;
}

/* returns -1 if failure */

int fread_ihx(FILE *fp, struct ihx *r)
{
	char line[1000];
	char * ptr = line;
	unsigned int record_len, record_addr, record_type, record_content,
		record_crc;
	int i;

	if (fgets(line,sizeof(line),fp) != line)
	{
		perror("fgets");
		return -1;
	}

	if (* ptr ++ != ':')
	{
		fprintf(stderr,"bad record\n");
		return -1;
	}

	ptr = get_byte(ptr, &record_len);
	if (ptr == NULL)
	{
		fprintf(stderr,"invalid record length\n");
		return -1;
	}

	ptr = get_word(ptr, &record_addr);
	if (ptr == NULL)
	{
		fprintf(stderr,"invalid record address\n");
		return -1;
	}

	ptr = get_byte(ptr, &record_type);
	if (ptr == NULL)
	{
		fprintf(stderr,"invalid record type\n");
		return -1;
	}

	if (record_type != TYPE_NORMAL && record_type != TYPE_EOF)
	{
		fprintf(stderr,"bad record type : %02X\n",record_type);
		return -1;
	}

	r->len = record_len;
	r->addr = record_addr;
	r->type = record_type;
	if (r->len != 0)
	{
		r->content = (unsigned char *)realloc(r->content, r->len);
		if (r->content == NULL)
		{
			perror("realloc");
			return -1;
		}
	}

	for (i=0;i<record_len;i++)
	{
		ptr = get_byte (ptr, &record_content);
		if (ptr == NULL)
		{
			fprintf(stderr,"invalid record content\n");
			return -1;
		}

		r->content[i] = record_content;
	}

	ptr = get_byte(ptr, &record_crc);
	if (ptr == NULL)
	{
		fprintf(stderr,"invalid record crc\n");
		return -1;
	}

	if (ihx_compute_crc(r) != record_crc)
	{
		fprintf(stderr,"record crc mismatch\n");
		return -1;
	}

	return 0;
}

/* returns -1 if failure */

int fwrite_ihx(FILE *fp, const struct ihx * r)
{
	int i;

	if (fprintf(fp,":%02X%04X%02X", r->len, r->addr, r->type) < 0)
		return -1;

	for (i=0;i<r->len;i++)
	{
		if (fprintf(fp,"%02X",r->content[i]) < 0)
			return -1;
	}

	if (fprintf(fp,"%02X\n",ihx_compute_crc(r)) < 0)
		return -1;

	return 0;
}

/* returns -1 if failure */

int ezusb_read_ihx(pusb_device_t dev, struct ihx *r,
				   unsigned int addr,
				   unsigned int len)
{
	int res;

	r->len = len;
	r->addr = addr;
	r->type = TYPE_NORMAL;
	if (len != 0)
	{
		r->content = (unsigned char *)realloc(r->content, len);
		if (r->content == NULL)
		{
			perror("realloc");
			return -1;
		}
	}

	res=pusb_control_msg(dev,0xc0,0xa0,addr,0,r->content, r->len,TIMEOUT);
	if (res != r->len)
	{
		perror("pusb_control_msg");
		return -1;
	}

	return 0;
}

/* returns -1 if failure */

int ezusb_write_ihx(pusb_device_t dev, const struct ihx *r)
{
	int res;

	res=pusb_control_msg(dev,0x40,0xa0,r->addr,0,r->content, r->len,TIMEOUT);
	if (res != r->len)
	{
		perror("pusb_control_msg");
		return -1;
	}

	return 0;
}

int ezusb_write_all(pusb_device_t dev, unsigned char b)
{
	unsigned char pkt[16];
	unsigned int addr, i;
	struct ihx ihx;
	int res = 0;

	ihx.len = sizeof(pkt);
	ihx.type = TYPE_NORMAL;
	ihx.content = pkt;

	for (i=0;i<sizeof(pkt);i++)
		pkt[i] = b;

	/* put the EZUSB in reset mode */
	if ((res=ezusb_write_ihx(dev, &ihx_reset_in)) < 0)
	{
		fprintf(stderr,"failed to reset in EZUSB\n");
		return -1;
	}

	for (addr=0; addr<0x10000; addr += sizeof(pkt))
	{
		ihx.addr = addr;

		for (i=0;i<sizeof(pkt);i++)
			pkt[i] = b + addr + i;

		if ((res=ezusb_write_ihx(dev, &ihx)) < 0)
		{
			fprintf(stderr,"failed writing block at %04x\n",addr);
			break;
		}
	}

	return res;
}

int fread__ezusb_write(pusb_device_t dev, const char *filename)
{
	FILE * fp;
	long size;
	struct ihx ihx;
	int res ;
	int line = 0;

	/* put the EZUSB in reset mode */
	if ((res=ezusb_write_ihx(dev, &ihx_reset_in)) < 0)
	{
		fprintf(stderr,"failed to reset in EZUSB\n");
		return -1;
	}

	/* open file for reading */
	fp = fopen(filename,"rb");
	if (fp == NULL)
	{
		perror(filename);
		return -1;
	}

	/* compute the file size */

	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	ihx_init(&ihx);

	/* loop through the file */
	while (ftell(fp) < size)
	{
		line ++;

		if ((res=fread_ihx(fp,&ihx)) < 0)
		{
			fprintf(stderr,"failed reading record %d\n",line);
			break;
		}

		if (ihx.type == TYPE_EOF)
			break;

		if ((res=ezusb_write_ihx(dev, &ihx)) < 0)
		{
			fprintf(stderr,"failed writing record %d\n",line);
			break;
		}
	}

	/* close file */

	fclose(fp);
	ihx_done(&ihx);

	/* put the EZUSB out reset mode */
	if ((res=ezusb_write_ihx(dev, &ihx_reset_out)) < 0)
	{
		fprintf(stderr,"failed to reset out EZUSB\n");
		fclose (fp);
		return -1;
	}

	return res;
}

int ezusb_read__fwrite(pusb_device_t dev, const char * filename)
{
	FILE * fp;
	unsigned int addr;
	struct ihx ihx;
	int res;
	int line = 0;

	/* open the file for writing */

	fp = fopen(filename,"wb");
	if (fp == NULL)
	{
		perror(filename);
		return -1;
	}

	ihx_init(&ihx);

	/* loop through the 64KB memory space,
	   by 16 bytes step */

	for (addr=0;addr<0x10000;addr += 16)
	{
		line ++;

		if ((res=ezusb_read_ihx(dev, &ihx, addr, 16)) < 0)
		{
			fprintf(stderr,"failed reading record %d\n",line);
			break;
		}

		if ((res=fwrite_ihx(fp, &ihx)) < 0)
		{
			fprintf(stderr,"failed writing record %d\n",line);
			break;
		}
	}

	/* write an end of file record */
	ihx.len  = 0;
	ihx.addr = 0;
	ihx.type = TYPE_EOF;

	res=fwrite_ihx(fp, &ihx);

	/* close file */
	fclose (fp);
	ihx_done(&ihx);

	return res;
}

void usage()
{
	printf("Usage: ezusb-mem r/w VID PID file.ihx\n");
	printf("  r : read the whole EZUSB memory and write to file.ihx\n");
	printf("  w : write the EZUSB memory with the content of file.ihx\n");
	exit (-1);
}

int main(int argc, char *argv[])
{
	int vid, pid;
	char f;
	const char * filename;
	pusb_device_t dev;

	/* parse command line option */

	if (argc != 5)
		usage();
	
	if (strcmp(argv[1],"r") != 0
		&& strcmp(argv[1],"w") != 0
		&& strcmp(argv[1],"0") != 0
		&& strcmp(argv[1],"1") != 0)
		usage();
	
	f = argv[1][0];

	vid = strtoul(argv[2],NULL,0);
	pid = strtoul(argv[3],NULL,0);

	filename = argv[4];

	/* open the USB device */
	dev = pusb_search_open(vid,pid);
	if (dev == NULL)
	{
		printf("I cannot find your EZ-USB device\n");
		return -1;
	}

	/* call the specified function : read or write */

	if (f == 'r')
		ezusb_read__fwrite(dev,filename);
	else if (f == 'w')
		fread__ezusb_write(dev,filename);
	else if (f == '0')
		ezusb_write_all(dev,0);
	else if (f == '1')
		ezusb_write_all(dev,0xff);

	/* close the USB device */

	pusb_close(dev);

	return 0;
}
