#include "eoc.c"

unsigned char buffer[] = { 
	0x0c, 0x0c, 0xF3, 0x43 ,
	0x0c, 0x0c, 0xF3, 0x43 ,
	0x0c, 0x0c, 0xF3, 0x43 ,
	0x0c, 0x0c, 0xF3, 0x43 
};
unsigned char buffer01[] = { 
0xf0 , 0x19 , 0x00 , 0x00 , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c,
0xf3 , 0x43 , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c,
0x0c , 0x0c , 0x0c , 0x0c , 0x00 , 0x80 , 0x00 , 0x80 , 0x9f , 0x63 , 0x00 , 0x2f , 0x63 , 0x00 , 0x0f , 0x62,
0x00 , 0x5f , 0x60 , 0x00 , 0x5f , 0x35 , 0x00 , 0xff , 0x5e , 0x00 , 0x0f , 0x37 , 0x00 , 0xdf , 0x5d , 0x00};

unsigned char buffer02[] = { 
0xf0 , 0x21 , 0x00 , 0x00 , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c,
0xf3 , 0x43 , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c , 0x0c,
0x0c , 0x0c , 0x0c , 0x0c , 0x00 , 0x80 , 0x00 , 0x80 , 0x9f , 0x63 , 0x00 , 0x2f , 0x63 , 0x00 , 0x0f , 0x62,
0x00 , 0x5f , 0x60 , 0x00 , 0x5f , 0x35 , 0x00 , 0xff , 0x5e , 0x00 , 0x0f , 0x37 , 0x00 , 0xdf , 0x5d , 0x00};

static unsigned char outbuf[32];

#define PRINTCHAR(X) \
	if ((X) >= ' ' && (X) < 0x7f) \
		printf("%c", (X)); \
	/* \
	else if ((X) == '\r' || (X) == '\n' || (X) == '\t' || (X) == '\b')  \
		printf("%c", (X)); \
	*/ \
	else \
		printf(".");

void dump(unsigned char* buf, int len)
{
	int i, j;

	for (i = 0; i < len; i += 16)
	{
		for (j = i; j < len && j < i + 16; j++)
			printf("%02x ", buf[j]);
		for (; j < i + 16; j++)
			printf("   ");
		for (j = i; j < len && j < i + 16; j++)
			PRINTCHAR(buf[j])
		printf("\n");
	}
}

int main()
{
	eoc_init();
	parse_eoc_buffer(buffer, 16);
	get_oec_answer(outbuf)
	dump(outbuf,32)

	eoc_init();
	parse_eoc_buffer(buffer01+4, 40);
	get_oec_answer(outbuf)
	dump(outbuf,32)

	parse_eoc_buffer(buffer02+4, 40);
	get_oec_answer(outbuf)
	dump(outbuf,32)

}

