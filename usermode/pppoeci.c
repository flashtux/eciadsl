/*
  Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 29/11/2001

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
*********************************************************************

  Design of a new way to handle endpoint for the ECI USB ADSL modem
  and replace pppoa2.c

  Design:

  1 process. using sig handler for SIGRTMIN
  20 ISO URB submitted
  1  ISO int submitted
  
  for (;;)
  {
    read (pppfd)
	write (endpoint 0x02)
  }

  in sig handler :
    URB = reapurb();
	if (urb = ep_int)
	{
	  compute some data
	  may sent vendor device 0xdd
	}
	if (urb = iso_ep)
    {
	   compute AAL5 frame
	   send to PPP over stdout
	}
   re-submit (same URB)


   03/12/2001 : Sometime, it ends via SEGV :-(
   Apparently, SEGV is caused by mixing call to malloc()/free()
   from the sig handler and outside.

(gdb) where
#0  0x40066806 in free () from /lib/libc.so.6
#1  0x40066558 in malloc () from /lib/libc.so.6
#2  0x40065d3e in malloc () from /lib/libc.so.6
#3  0x804b0cd in pusb_device_get_urb (dev=0x8100600) at pusb-linux.c:536
#4  0x8049c24 in sig_rtmin (sig=32) at pppoeci.c:862
#5  0x40033008 in sigaction () from /lib/libc.so.6
#6  0x40065d3e in malloc () from /lib/libc.so.6
#7  0x8049446 in aal5_write (epdata=0x8100630, buf=0x804d502 "", n=86)
    at pppoeci.c:532
#8  0x804985c in handle_endpoint_02 (epdata=0x8100630, fdin=3) at pppoeci.c:693
#9  0x804a494 in main (argc=7, argv=0xbffff924) at pppoeci.c:1156
#10 0x4002ccbe in __libc_start_main () from /lib/libc.so.6

    04/12/2001 : malloc() has been removed in this file.

	22/12/2001 : remove the #undef N_HDLC. N_HDLC is needed!!! if you don't
	use it, you will have severe upload problems. You may need to recompile
    the n_hdlc kernel module and apply a patch (the instructions are the
	same for the speedtouch drivers => http://speedtouch.sourceforge.net/)

	26/12/2001 : Change the code around the select() loop. Timeout has been
	set to 1ms (previously 0.5ms).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>		/* N_HDLC & TIOCSETD */
#include <sys/resource.h>	/* setpriority() */
#include <string.h>
#include <sched.h>		/* for sched_setscheduler */
#include <limits.h>		/* for LONG_MAX */
#include <sys/types.h>
#include <sys/socket.h>

/* my USB library */
#include "pusb.h"

#define PAD_SIZE 11
/*	For debugin only	*/
void dump_urb(char *buffer,int size);

/* for ident(1) command */
static char id[] =
	"@(#) $Id$";

/* USB level */
#define DATA_TIMEOUT 5000
#include "modem.h"

/* max size of data endpoint (Windows driver uses 448). */
#define MAX_EP_SIZE 448
/* number of simultaneous URB submitted to data endpoint (Windows drv : 20 */
#define NB_PKT_EP_DATA_IN 20
/* number of ISO packet per URB (Windows driver uses 10) */
#define PKT_NB 10

/* ATM level : adapt according your ADSL provider settings */
int my_vpi = -1;
int my_vci = -1;

#define CELL_SIZE 53
#define CELL_HDR  05
#define CELL_DATA 48

/* PPP level : DO NOT MODIFY */
#define syncHDLC 1

/*global parameters*/
int   verbose = 0;
int   usb_sync  = 1;
pusb_device_t fdusb;
/*unsigned char lbuf[20][4400];*/
pusb_endpoint_t ep_data_out,ep_data_in, ep_int;
pid_t this_process;		/*always the current   process pid*/

/* predeclarations */

int aal5_read(unsigned char *cell_buf, size_t count,
			  unsigned int vpi, unsigned int vci, unsigned int pti, int fdout);

/*
* variable that will be used in several function
*   => so global
*/



int gfdout;

#define CRC32_REMAINDER CBF43926
#define CRC32_INITIAL 0xffffffff

#define CRC32(c,crc) (crc32tab[((size_t)(crc>>24) ^ (c)) & 0xff] ^ (((crc) << 8)))

unsigned long crc32tab[256] = {
	0x00000000L, 0x04C11DB7L, 0x09823B6EL, 0x0D4326D9L,
	0x130476DCL, 0x17C56B6BL, 0x1A864DB2L, 0x1E475005L,
	0x2608EDB8L, 0x22C9F00FL, 0x2F8AD6D6L, 0x2B4BCB61L,
	0x350C9B64L, 0x31CD86D3L, 0x3C8EA00AL, 0x384FBDBDL,
	0x4C11DB70L, 0x48D0C6C7L, 0x4593E01EL, 0x4152FDA9L,
	0x5F15ADACL, 0x5BD4B01BL, 0x569796C2L, 0x52568B75L,
	0x6A1936C8L, 0x6ED82B7FL, 0x639B0DA6L, 0x675A1011L,
	0x791D4014L, 0x7DDC5DA3L, 0x709F7B7AL, 0x745E66CDL,
	0x9823B6E0L, 0x9CE2AB57L, 0x91A18D8EL, 0x95609039L,
	0x8B27C03CL, 0x8FE6DD8BL, 0x82A5FB52L, 0x8664E6E5L,
	0xBE2B5B58L, 0xBAEA46EFL, 0xB7A96036L, 0xB3687D81L,
	0xAD2F2D84L, 0xA9EE3033L, 0xA4AD16EAL, 0xA06C0B5DL,
	0xD4326D90L, 0xD0F37027L, 0xDDB056FEL, 0xD9714B49L,
	0xC7361B4CL, 0xC3F706FBL, 0xCEB42022L, 0xCA753D95L,
	0xF23A8028L, 0xF6FB9D9FL, 0xFBB8BB46L, 0xFF79A6F1L,
	0xE13EF6F4L, 0xE5FFEB43L, 0xE8BCCD9AL, 0xEC7DD02DL,
	0x34867077L, 0x30476DC0L, 0x3D044B19L, 0x39C556AEL,
	0x278206ABL, 0x23431B1CL, 0x2E003DC5L, 0x2AC12072L,
	0x128E9DCFL, 0x164F8078L, 0x1B0CA6A1L, 0x1FCDBB16L,
	0x018AEB13L, 0x054BF6A4L, 0x0808D07DL, 0x0CC9CDCAL,
	0x7897AB07L, 0x7C56B6B0L, 0x71159069L, 0x75D48DDEL,
	0x6B93DDDBL, 0x6F52C06CL, 0x6211E6B5L, 0x66D0FB02L,
	0x5E9F46BFL, 0x5A5E5B08L, 0x571D7DD1L, 0x53DC6066L,
	0x4D9B3063L, 0x495A2DD4L, 0x44190B0DL, 0x40D816BAL,
	0xACA5C697L, 0xA864DB20L, 0xA527FDF9L, 0xA1E6E04EL,
	0xBFA1B04BL, 0xBB60ADFCL, 0xB6238B25L, 0xB2E29692L,
	0x8AAD2B2FL, 0x8E6C3698L, 0x832F1041L, 0x87EE0DF6L,
	0x99A95DF3L, 0x9D684044L, 0x902B669DL, 0x94EA7B2AL,
	0xE0B41DE7L, 0xE4750050L, 0xE9362689L, 0xEDF73B3EL,
	0xF3B06B3BL, 0xF771768CL, 0xFA325055L, 0xFEF34DE2L,
	0xC6BCF05FL, 0xC27DEDE8L, 0xCF3ECB31L, 0xCBFFD686L,
	0xD5B88683L, 0xD1799B34L, 0xDC3ABDEDL, 0xD8FBA05AL,
	0x690CE0EEL, 0x6DCDFD59L, 0x608EDB80L, 0x644FC637L,
	0x7A089632L, 0x7EC98B85L, 0x738AAD5CL, 0x774BB0EBL,
	0x4F040D56L, 0x4BC510E1L, 0x46863638L, 0x42472B8FL,
	0x5C007B8AL, 0x58C1663DL, 0x558240E4L, 0x51435D53L,
	0x251D3B9EL, 0x21DC2629L, 0x2C9F00F0L, 0x285E1D47L,
	0x36194D42L, 0x32D850F5L, 0x3F9B762CL, 0x3B5A6B9BL,
	0x0315D626L, 0x07D4CB91L, 0x0A97ED48L, 0x0E56F0FFL,
	0x1011A0FAL, 0x14D0BD4DL, 0x19939B94L, 0x1D528623L,
	0xF12F560EL, 0xF5EE4BB9L, 0xF8AD6D60L, 0xFC6C70D7L,
	0xE22B20D2L, 0xE6EA3D65L, 0xEBA91BBCL, 0xEF68060BL,
	0xD727BBB6L, 0xD3E6A601L, 0xDEA580D8L, 0xDA649D6FL,
	0xC423CD6AL, 0xC0E2D0DDL, 0xCDA1F604L, 0xC960EBB3L,
	0xBD3E8D7EL, 0xB9FF90C9L, 0xB4BCB610L, 0xB07DABA7L,
	0xAE3AFBA2L, 0xAAFBE615L, 0xA7B8C0CCL, 0xA379DD7BL,
	0x9B3660C6L, 0x9FF77D71L, 0x92B45BA8L, 0x9675461FL,
	0x8832161AL, 0x8CF30BADL, 0x81B02D74L, 0x857130C3L,
	0x5D8A9099L, 0x594B8D2EL, 0x5408ABF7L, 0x50C9B640L,
	0x4E8EE645L, 0x4A4FFBF2L, 0x470CDD2BL, 0x43CDC09CL,
	0x7B827D21L, 0x7F436096L, 0x7200464FL, 0x76C15BF8L,
	0x68860BFDL, 0x6C47164AL, 0x61043093L, 0x65C52D24L,
	0x119B4BE9L, 0x155A565EL, 0x18197087L, 0x1CD86D30L,
	0x029F3D35L, 0x065E2082L, 0x0B1D065BL, 0x0FDC1BECL,
	0x3793A651L, 0x3352BBE6L, 0x3E119D3FL, 0x3AD08088L,
	0x2497D08DL, 0x2056CD3AL, 0x2D15EBE3L, 0x29D4F654L,
	0xC5A92679L, 0xC1683BCEL, 0xCC2B1D17L, 0xC8EA00A0L,
	0xD6AD50A5L, 0xD26C4D12L, 0xDF2F6BCBL, 0xDBEE767CL,
	0xE3A1CBC1L, 0xE760D676L, 0xEA23F0AFL, 0xEEE2ED18L,
	0xF0A5BD1DL, 0xF464A0AAL, 0xF9278673L, 0xFDE69BC4L,
	0x89B8FD09L, 0x8D79E0BEL, 0x803AC667L, 0x84FBDBD0L,
	0x9ABC8BD5L, 0x9E7D9662L, 0x933EB0BBL, 0x97FFAD0CL,
	0xAFB010B1L, 0xAB710D06L, 0xA6322BDFL, 0xA2F33668L,
	0xBCB4666DL, 0xB8757BDAL, 0xB5365D03L, 0xB1F740B4L
};

unsigned int
calc_crc(unsigned char *mem, int len, unsigned int initial)
{

	unsigned int crc;

	crc = initial;

	for(; len; mem++, len--)
		crc = CRC32(*mem, crc);


	return (crc);

}

/*
* display a convenient string about the current time.
* the line printed is not \n terminated
*/

void
print_time()
{

	struct timeval tv;

	gettimeofday(&tv, NULL);

	printf("[%.24s %6ld]", ctime(&tv.tv_sec), tv.tv_usec);

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
		fflush(NULL) ;

	}
}

/*
* ppp_read returns a pointer to a static buffer.
* it returns the number of bytes read or -1 in case of errors.
* 0 should indicates that pppd exited (not tested).
*/

int
ppp_read(int fd, unsigned char **buf, int *n)
{

	/*
	* we must handle HDLC framing, since this is what pppd
	* sends to us. I use some code from rp-pppoe
	*/

	static unsigned char ppp_buf[64 * 1024];
	int r;

	for(;;) {

		do {
			r = read(fd, ppp_buf, sizeof(ppp_buf));
		} while(r < 0 && errno == EINTR);

		if(r < 0 && verbose) {
			fprintf(stderr, "< pid=%d > Error reading from pppd, see the reason just below\n", this_process);
			perror("Reason");
		}

		/*Debug message*/
		if(verbose>1) {
			printf("< pid=%d > ", this_process);
			print_time();
			printf(" Read from ppp, len = %d\n", r);
			dump(ppp_buf, r);
		}

		if(r <= 0)
			return r;

		/* supress leading 2 bytes */

		if(syncHDLC) {

			if(r < 3 && verbose) {
				printf("< pid=%d > ", this_process);
				print_time();
				printf(" Read from ppp short on data, r=%d => ignored\n", r);
				continue;
			}

			if(r == sizeof(ppp_buf) && verbose) {
				printf("< pid=%d > ", this_process);
				print_time();
				printf(" Read from ppp too much data, r=%d => ignored\n", r);
				continue;
			}

			*buf = ppp_buf + 2;
			*n = r - 2;

			return(r - 2);

		}
		else {

			if(verbose) printf("< pid=%d > async HDLC not handled\n", this_process);

		}

		break;

	}

}

#define NOBUF_RETRIES 5

int
ppp_write(int fd, unsigned char *buf, int n)
{
	unsigned char ppp_buf[64 * 1024];
	int r;
	static int errs = 0;
	int retries = 0;

	if(syncHDLC) {
		ppp_buf[0] = 0xff;	//FRAME_ADDR;
		ppp_buf[1] = 0x03;	//FRAME_CTRL;

		memcpy(ppp_buf + 2, buf, n);

		if(verbose>1) {
			printf("< pid=%d > ", this_process);
			print_time();
			printf(" writing to ppp, len = %d\n", n + 2);
			dump(ppp_buf, n + 2);
		}

	retry:

		if((r = write(fd, ppp_buf, n + 2)) < 0) {

			/*
			* We sometimes run out of mbufs doing simultaneous
			* up- and down-loads on FreeBSD.  We should find out
			*  why this is, but for now...
			*/

			if(errno == ENOBUFS) {

				errs++;

				if((errs < 10 || errs % 100 == 0) && verbose)
					fprintf(stderr, "< pid=%d > ppp_write: %d ENOBUFS errors\n", this_process, errs);

				if(retries++ < NOBUF_RETRIES) {
					/* retry after delay */
					sleep(1);
					goto retry;
				}
				else { /* throw away the packet */
					return(0);
				}

			}
			else {
				if(verbose) {
					fprintf(stderr, "< pid=%d > Error writing to pppd, see the reason below\n", this_process);
					perror("Reason");
				}
			}
		}
		return(r);
	}
	else {
		if(verbose) printf("< pid=%d > async HDLC not handled\n", this_process);
	}

	return(-1);
}

/*
* cell_read : is used to decode an individual cell and to try to make
* an AAL5 frame.
*/

int
cell_read(unsigned char *cellbuf, size_t count, int fdout)
{

	unsigned int vpi, vci, pti;

	/* decoding & printing of the cell */
	vpi = ( cellbuf[0]         <<  4) | (cellbuf[1] >> 4);
	vci = ((cellbuf[1] & 0x0f) << 12) | (cellbuf[2] << 4) | (cellbuf[3] >> 4);
	pti = ( cellbuf[3] & 0x0f);

	/* filling our AAL5 frame */
	return(aal5_read(cellbuf + CELL_HDR, CELL_DATA, vpi, vci, pti, fdout));
}

int
cell_write(unsigned char *cellbuf,
			 unsigned char *buf, int n,
			 unsigned int vpi, unsigned int vci, unsigned int pti)
{

	unsigned char gfc = 0;
	unsigned char hec = 0x00; /* change to the value used in the Windows driver */

	if(n != CELL_DATA)
		return(-1);

	/* normalize */
	vpi &= 0xff;
	vci &= 0xffff;
	pti &= 0x0f;

	cellbuf[0] = (gfc << 4) | (vpi >> 4);
	cellbuf[1] = (vpi << 4) | (vci >> 12);
	cellbuf[2] = (vci >> 4);
	cellbuf[3] = ((vci & 0xf) << 4) | pti;
	cellbuf[4] = hec;

	memcpy(cellbuf + CELL_HDR,  buf, CELL_DATA);
	memset(cellbuf + CELL_SIZE,0xff, PAD_SIZE);

	return(CELL_SIZE + PAD_SIZE);
}

/* 
* parse an ATM cell and try to complete (or make a new one)
* AAL5 frame. If succes, the frame is sent to pppd. If error, -1 is returned
* (0 otherwise)
*/

int
aal5_read(unsigned char *cell_buf, size_t count,
		unsigned int vpi, unsigned int vci, unsigned int pti, int fdout)
{

	static unsigned char buf[64 * 1024];
	static int len = 0;

	int ret;
	unsigned int real_len;
	unsigned int crc1, crc2;

	if(vpi != my_vpi || vci != my_vci) {
/*
		if(verbose) {
			printf("< pid=%d > ", this_process);
			print_time();
			printf(" vpi.vci mismatch (%d/%d instead of %d/%d) ... ignored!\n",
				 vpi, vci, my_vpi, my_vci);
			dump(cell_buf, CELL_DATA);
		}
*/
		return(0);
	
	}

	/*
	* We should add a check for OAM cells, which should be done before
	* copying the cell's content to our buffer
	*/

	if(len + CELL_DATA < sizeof(buf)) {

		memcpy(buf + len, cell_buf, CELL_DATA);
		len += CELL_DATA;

	}
	else {
		if(verbose) {
			printf("< pid=%d > ", this_process);
			print_time();
			printf(" aal5_read: buffer overflow (len=%d) => reseting buffer\n", len);
		}

		len = 0;
		return(0);

	}

	/* check if PTI=1, which indicated last cell of an AAL5 frame */
	if((pti & 2) == 0)
		return(0);

	real_len = (buf[len - 6] << 8) | buf[len - 5];

	if(verbose>1)
		printf("< pid=%d > aal5_read: len=%d => real_len=%d\n", this_process, len, real_len);

	/*
	  Check the CRC of received AAL5 frames. 
	  crc1 : computed crc on the current AAL5 frame (the theoretical value)
	  crc2 : crc read from the AAL5 frame (the real value)
	*/

	crc1 = ~calc_crc(buf, len -4, -1);
	crc2 = (buf[len-4] << 24) | (buf[len-3] << 16)
		| (buf[len-2]<<8) | buf[len-1];

	if (crc1!=crc2 && !(verbose>1))
	{
		printf("< pid=%d > ", this_process);
		print_time();
		printf(" aal5_read: len=%d => real_len=%d\n",len, real_len);
	}
		
	if (crc1 != crc2)
	{
		printf("< pid=%d > ",this_process);
		print_time();
		printf(" bad crc %08x (instead of %08x) => dropping AAL5 frame\n",
			   crc2,crc1);
		
		len = 0;
		return 0;
	}

	ret = ppp_write(fdout, buf, real_len);
	len = 0;

	return ret;
}

/*
* Warning: (ok, tooks me half an hour to find my own buf)
* buf must be large enough to contains the whole AAL5 frame,
* with its layer.
*/

int
aal5_write(pusb_endpoint_t epdata, unsigned char *buf, int n)
{
	/* add the trailer */
	int total_len = 48 * ((n + 8 + CELL_DATA - 1) / 48);
	unsigned int crc, pti = 0;

	/*
	* this buffer will contain all cells for an AAL5 frame,
	* so AAL5 frame is max 64K + 48 that's 1367 cells at max
	*/

	unsigned char bigbuf[1367 * 53];
	int ptr;
	int ret;
/*
	bigbuf = (unsigned char *)malloc(1367 * 53);
	if (bigbuf == NULL)
		return -1;
*/

	memset(buf + n, 0, total_len - n);

	buf[total_len - 6] = n >> 8;
	buf[total_len - 5] = n & 0xff;

	crc = ~calc_crc(buf, total_len - 4, -1);

	buf[total_len - 4] = crc >> 24;
	buf[total_len - 3] = crc >> 16;
	buf[total_len - 2] = crc >> 8;
	buf[total_len - 1] = crc;

	ptr = 0;
	while(total_len > 0) {

		if(total_len <= CELL_DATA)
			/* this is the last cell, so mark it */
			pti = 2;

		ptr += cell_write(bigbuf + ptr, buf, 
				CELL_DATA, my_vpi, my_vci, pti);

		/* send the same cell over another vpi/vci */

		buf += CELL_DATA;
		total_len -= CELL_DATA;

	}

	/*Debud message*/
	if(verbose>1) {
		printf("< pid=%d > ", this_process);
		print_time();
		printf(" writing to usb, len = %d\n", ptr);
		dump(bigbuf, ptr);
	}

/*
	ret = pusb_endpoint_write(epdata, bigbuf, ptr,DATA_TIMEOUT);
*/


	ret = pusb_endpoint_submit_write (epdata,bigbuf,ptr,0); // SIGRTMIN);
	
	if(ret < 0 && verbose) {
		fprintf(stderr, "< pid=%d > Error writing to usb, see the reason\n", this_process);
		perror("Reason");
	}

	return(ret);

}
/* returns -1 in case of errors */

int
decode_usb_pkt(unsigned char *buf, int len)
{
	int r;
	static unsigned char rbuf [ 64*1024];
	static int rlen = 0;
	int pos;
	static int waiting_for_53 = 0;

	/*Debug message*/
	if(verbose>1) {
		printf("< pid=%d > ", this_process);
		print_time();
		printf(" read from usb, len = %d\n", len);
		dump(buf, len);
	}

	/*if (len == MAX_EP_SIZE )
	{
		printf("%d bytes? eh eh ... I'm waiting for a packet with a multiple of 53 bytes, sir!\n", MAX_EP_SIZE);
		waiting_for_53 = 1;
		return 0;
	}

	if (waiting_for_53)
	{
		if ((len % 53) != 0)
		{
			printf("sorry ... ignoring packet\n");
			return 0;
		}
		else
		{
			printf("ok... back to normal :-)\n");
			waiting_for_53 = 0;
		}
	}

	if ((len % 53) == 0)
	{
		if (rlen != 0)
		{
			printf("dropping %d bytes\n",rlen);
			rlen = 0;
		}
	}*/

	/* add the received buf to our buf, if possible */

	if (rlen + len < sizeof(rbuf))
	{
		memcpy(rbuf + rlen, buf, len);
		rlen += len;
	}
	else
	{
		printf("< pid=%d > ",this_process);
		print_time();
		printf("warning: no space for %d bytes\n",len);
	}

	pos = 0;
	while((rlen - pos) >= CELL_SIZE) {

		if((r = cell_read(rbuf + pos, CELL_SIZE, gfdout)) < 0)
		{
			rlen -= pos;
			if (rlen != 0)
				memcpy(rbuf, rbuf+pos, rlen);
			return(r);
		}
		pos += CELL_SIZE;
	};
	rlen -= pos;

	if (rlen != 0)
		memcpy(rbuf, rbuf + pos, rlen);

	return(0);
}

void
usage()
{

	fprintf(stderr, "pppoeci version $Name$\n");
	fprintf(stderr, "usage: pppoeci



 [-v val] [-f logfile] -vpi val -vci val\n");
	fprintf(stderr, "  -v       : defines the verbosity level [0-2] ( Enables logging )\n");
	fprintf(stderr, "  -f       : defines the log filename to use ( Default /tmp/pppoa2.log )\n");
	fprintf(stderr, "  -vpi.vci : defined the vpi.vci that your provider is\n");
	fprintf(stderr, "             using, 8.35 for France, 0.38 for UK\n");
	fprintf(stderr, "  --help   : this message\n");
	exit(-1);

}

/*
  init_ep_int: submit one URB to read the interrupt endpoint
*/

int init_ep_int(pusb_endpoint_t ep_int)
{
	/*
	  The buffer needs to be static, since its kept by
	  pusb_endpoint_submit_read. We use a buffer size of 0x40 (64)
	  like the Windows driver. If we we using a buffer of size 100 bytes,
	  we would receive 100 bytes instead... 
	*/

#define NB_PKT_EP_INT 64
	static unsigned char buf[NB_PKT_EP_INT][0x40];
	int i,ret;
   
	for (i=0;i<NB_PKT_EP_INT;i++)
	{
		/* we use a BULK transfer instead, since INTERRUPT transfer
		   are not supported by usbdevfs (devio.c) */
		ret = pusb_endpoint_submit_int_read(ep_int,buf[i],
						    sizeof(buf[i]), 0); // SIGRTMIN);
		if (ret < 0)
		{
			perror("error: init_ep_int");
			return ret;
		}
	}

	return 0;
}

int init_ep_data_in(pusb_endpoint_t ep_data_in)
{
	static unsigned char buf[NB_PKT_EP_DATA_IN][MAX_EP_SIZE*PKT_NB];
	int i, ret;

	for (i=0;i<NB_PKT_EP_DATA_IN;i++)
	{
		ret = pusb_endpoint_submit_iso_read(ep_data_in,buf[i],MAX_EP_SIZE,
						    PKT_NB,0); //SIGRTMIN);
		if (ret < 0)
		{
			perror("error: init_ep_data_in");
			return ret;
		}
	}

	return 0;
}

void replace_b1_b2(unsigned char *b1, unsigned char *b2)
{
	/* we should return :
	   0x11 0x11 0x13 0x13 0x13 0x13 0x11 0x11 0x01 0x01 0x01 0x01
	   0x11 0x11 0x13 0x13 0x13 0x13 0x11 0x11 0x53 0x53 0x53 0x53 
	   The last line is repeated infinitely
	*/
	
	static int count = 0;

	static unsigned char replace_b1[] = {
		0x73, 0x73, 0x63, 0x63, 0x63, 0x63, 0x73, 0x73, 0x63, 0x63, 0x63, 0x63
	};

	static unsigned char replace_b2[] = {
		0x11, 0x11, 0x13, 0x13, 0x13, 0x13, 0x11, 0x11, 0x01, 0x01, 0x01, 0x01
	};

	*b1 = replace_b1[count];
	*b2 = replace_b2[count];

	count = (count + 1) % 12;

	if (count == 0)
		replace_b2[8] = replace_b2[9] = replace_b2[10] = replace_b2[11] = 0x53;
}

void handle_ep_int(unsigned char * buf, int size, pusb_device_t fdusb)
{
	unsigned char outbuf[40] = {
		0xff, 0xff, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c, 
		0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c, 
		0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c, 
		0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c, 
		0x0c, 0x0c
	};

	int i, outi = 0;

	if (verbose>1)
	{
		printf("< pid=%d > ", this_process);
		print_time();
		printf(" read from ep int, len = %d\n", size);
		dump(buf, size);
	}

	for (i=0;i<15;i++)
	{
		unsigned char b1, b2;

		b1 = buf[6+2*i+0];
		b2 = buf[6+2*i+1];

		/* each word != 0x0c 0x0c is copied in the output buffer */

		if (b1 != 0x0c || b2 != 0x0c)
		{
			/* however 0x7311 need to be replace by
			   0x6313, 0x6301, 0x6313, 0x6353 ...  */

			if (b1 == 0x73 && b2 == 0x11)
				replace_b1_b2(&b1,&b2);


			/* we check that we are not writing outside our buffer.
			   From what is found in our log, this never happen.
			*/

			if (10 + outi >= sizeof(outbuf))
			{
				printf("< pid=%d > ", this_process);
				print_time();
				printf(" warning: outbuf is too small\n");
				dump(buf,size);
				break;
			}

			outbuf[10 + outi++] = b1;
			outbuf[10 + outi++] = b2;
		}
	}

	if (outi != 0)
	{
/*
		printf("vendor device buf, len = %d\n",sizeof(outbuf));
		dump(outbuf,sizeof(outbuf));
*/	
		if (pusb_control_msg(fdusb,0x40,0xdd,0xc02,0x580,outbuf,
							 sizeof(outbuf),DATA_TIMEOUT) != sizeof(outbuf))
		{
			perror("error: can't send vendor device!!!");
		}
	}
}

void handle_ep_data_in(unsigned char * buf, int size)
{
	decode_usb_pkt(buf,size);
}

void handle_urb(pusb_urb_t urb)
{
	unsigned char * buf, * sbuf;
	int idx, size, ret;

	switch (pusb_urb_get_epnum(urb))
	{
	case EP_INT:
		/*printf("interrupt !!!\n");*/

		if (pusb_urb_buffer_first(urb,&buf,&size,&idx))
		{
			do {
/*
				printf("interrupt %02x, packet len = %d\n",
					   EP_INT,size);
				dump(buf,size);
*/
				handle_ep_int(buf,size, fdusb);

			} while (pusb_urb_buffer_next(urb,&buf,&size,&idx));
		
			/* we use the last buffer & size. And since, there is ONLY
			   one buffer in INT transfer, this works */
			ret = pusb_endpoint_submit_int_read(ep_int, buf, 0x40, 0); //SIGRTMIN);
			if (ret < 0)
				perror("error: can re-submit on EP_INT");
		}
		break;
		
	case EP_DATA_IN:
		/*printf("data in !!!\n");*/
		
		if (pusb_urb_buffer_first(urb,&buf,&size,&idx))
		{
			sbuf = buf;
	
			do {
				if (size != 0)
				{
/*
					printf("data_in %02x, packet len = %d\n",
						   EP_DATA_IN,size);
					dump(buf,size);
*/
					handle_ep_data_in(buf,size);
				}
			} while (pusb_urb_buffer_next(urb,&buf,&size,&idx));

			ret=pusb_endpoint_submit_iso_read(ep_data_in,sbuf,MAX_EP_SIZE,
							  PKT_NB,0); //SIGRTMIN);
			if (ret < 0)
				perror("error: can't re-submit on ep EP_DATA_IN");
		}
		break;

	case EP_DATA_OUT:
		/* ok, nothing to say */
		break;

	default:
		printf("error: unknown endpoint : %02x\n",pusb_urb_get_epnum(urb));
		break;
	}
}

void sig_rtmin(int sig)
{
	pusb_urb_t urb;
	int count;
	for (count = 0; ; count ++)
	{
		urb = pusb_device_get_urb(fdusb);
		if (urb == NULL)
			break;
		handle_urb(urb);

		/* very bad line : pusb_device_get_urb() returns a pointer
		   to a memory block allocated by malloc(), so we need to call free().
		   However, this is a bad design in general, since allocated memory
		   should be freed by the "module" that allocate it (ie pusb).
		   Anyway, its a quick fix.
		*/

		free (urb);
	}
}

/* 
* endpoint 0x02 is used for sending ATM cells
*
* to handle synchronous write, we does an infinite
* loop, keeping writing on endpoint EP_DATA_OUT
*
* NB : this function must be called after the
*      handle_endpoint_88 which is a fork.
*      handle_endpoint_02 is an infinite loop
*      so if you enter it, you will never get
*      out ( just when there are errors)
*/

void
handle_endpoint_02(pusb_endpoint_t epdata, int fdin,pusb_endpoint_t ep_data_in, pusb_endpoint_t ep_int)
{
	/* this part still need a redesign ... */
	int r, n;
	unsigned char *buf;

	fd_set inf;
	int select_rv;
	int maxfd;
	struct timeval tv;

	maxfd = fdin+1;

	for(;;)
	{
	  FD_ZERO(&inf);
	  FD_SET(fdin,&inf);
	  // Consider that tv is reseted by select
	  tv.tv_sec = 0;
	  tv.tv_usec = 1000; /* 1ms */

	  /*
		Benoit PAPILLAULT : The timeout is 1ms, USB is scheluded at 1ms
		rate. So we check for completed URB at this rate too. Note that
		the sig_rtmin() function should be called even if the select()
		returns without reaching the timeout.
	  */

	  // The usb fd does not seem to work in select... too bad !!!
	  // FD_SET(ep_int->fd,&inf); 
	  // ep_data_in is the same fd...
	  // FD_SET(ep_data_in->fd,&inf);

	  select_rv = select(maxfd, &inf, NULL, NULL, &tv);
	  if (select_rv > 0)
	  {
	    n = ppp_read(fdin, &buf, &n);

	    if(n <= 0)
	      {
			  printf("ppp_read=%d\n",n);
			  break;
	      }
	    
	    r = aal5_write(epdata, buf, n);
	    
	    if(r < 0)
		{
			printf("aal5_write=%d\n",r);
			break;
		}
	  }
	  else if (select_rv < 0)
	  {
		  /* probably pppd is terminating ... */
		  break;
	  }

	  sig_rtmin(32);
	}
}

int main(int argc, char *argv[])
{
	char *logfile;
	int   fdin, fdout, log;
	int   i;

	this_process = getpid();
	logfile = "/tmp/pppoa2.log";
	log = 0;

	for(i = 1; i < argc; i++) {

		if(strcmp(argv[i], "-v") == 0 && i + 1 < argc)
			verbose = atoi(argv[++i]);
		else if(strcmp(argv[i], "-vpi") == 0 && i + 1 < argc)
			my_vpi = atoi(argv[++i]);
		else if(strcmp(argv[i], "-vci") == 0 && i + 1 < argc)
			my_vci = atoi(argv[++i]);
		else if(strcmp(argv[i], "--help") == 0)
			usage();
		else if(strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
			logfile   = argv[++i];
			if(!verbose) verbose = 1;
		}
		else 
			usage();
	}

	if(my_vpi == -1 || my_vci == -1)
		usage();

	if(verbose)
		fprintf(stderr, "Using vpi.vci=%d.%d\n", my_vpi, my_vci);

	/*Duplicate in and out fd*/
	fdin  = dup(0);
	fdout = dup(1);

	/*
	* If the verbosity level is greater than 0
	* we have to create a log
	*/
	if(verbose) {

		log = open(logfile, O_CREAT | O_RDWR | O_APPEND, 0600);

		if(log < 0) {
			if(verbose) perror(logfile);
			return(-1);
		}

		for(i=0; i<8 ; i++)
			write(log, "##########",10);

		write(log, "\n\n",2);

	}

	/*Close all standard streams : input, output, error*/
	close(0);
	close(1);
	close(2);

	/*
	* Like the verbosity level is > 0
	* we will redirect all standard streams to the log file
	* so we are sure to catch all messages from this program :)
	*/
	if(verbose) {

		time_t ourtime;

		/*Duplicate log fd, so 0,1,2 point to the log file*/
		dup(log);
		dup(log);
		dup(log);

		/*We don't need log fd anymore*/
		close(log);

		/* no buffering on stdout & stderr : it doesn't seem to work */
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);

		time(&ourtime);
		printf("Log started on %s\n", ctime(&ourtime));

	}

	/*Debug messages*/
	if(verbose > 1) {
		printf("< pid=%d > : Hi I am the parent process, i handle the endpoint 0x07\n", getpid());

		printf("< pid=%d > : File descriptors : fdin=%d, fdout=%d\n", this_process, fdin, fdout);
	}



	/* Increase priority of the pppoa process*/
	if(setpriority(PRIO_PROCESS, this_process, -20) < 0 && verbose) {
		fprintf(stderr, "< pid=%d > setpriority, see the reason just below\n", this_process);
		perror("Reason");
	}

#if defined(__FreeBSD__) || defined(__NetBSD__)
#define SOCKBUF (64*1024)
	{
		int sbuf, ss = sizeof(sbuf);

		if(getsockopt(fdin, SOL_SOCKET, SO_SNDBUF, &sbuf, &ss) == 0) {

			/*Debug message*/
			if(verbose > 1)
				fprintf(stderr, "< pid=%d > Increasing SNDBUF from %d to %d\n", this_process, sbuf, SOCKBUF);

			sbuf = SOCKBUF;

			if(setsockopt(fdin, SOL_SOCKET, SO_SNDBUF, &sbuf, ss) < 0 && verbose) {
				fprintf(stderr,"< pid=%d > setsockopt, see the reason just below\n", this_process);
				perror("Reason");
			}

		}

		if(getsockopt(fdin, SOL_SOCKET, SO_RCVBUF, &sbuf, &ss) == 0) {

			/*Debug message*/
			if(verbose > 1)
				fprintf(stderr, "< pid=%d > Increasing RCVBUF from %d to %d\n", sbuf, SOCKBUF, this_process);

			sbuf = SOCKBUF;

			if(setsockopt(fdin, SOL_SOCKET, SO_RCVBUF, &sbuf, ss) < 0 && verbose) {
				fprintf(stderr, "< pid=%d > setsockopt, see the reason just below\n", this_process);
				perror("Reason");
			}

		}

	}
#endif

	/*
	* Install HDLC line discipline on fdin if it is a tty and
	* the OS has such a thing.
	*/

	if(isatty(fdin)) {
#ifdef N_HDLC
		int disc = N_HDLC;

		if(ioctl(fdin, TIOCSETD, &disc) < 0) {
			if(verbose) fprintf(stderr, "< pid=%d > Error loading N_HDLC\n", this_process);
			return(-1);
		}
#endif
	}

	/* 
	 * Install a SIGRTMIN signal handler. This signal is used for the 
	 * completion of an URB */

	//signal(SIGRTMIN, sig_rtmin);

	/*
	*  We search for the first USB device matching ST_VENDOR & ST_PRODUCT.
	*  usbdevfs must be mount on /proc/bus/usb (or you may change the path
	*  here, according to your config
	*/

	fdusb = pusb_search_open(ST_VENDOR, ST_PRODUCT);

	if(fdusb == NULL && verbose) {
		printf("< pid=%d > Where is this crappy modem ?!\n", this_process);
		return(-1);
	}

	/*Debug message*/
	if(verbose > 1)
		printf("< pid=%d > Got the modem, yipiyeah !\n", this_process);

	/* Initialize global variables */
	gfdout = fdout;

	/* We claim interface 1, where endpoints 0x02, 0x86 & 0x88 are */

	if(pusb_claim_interface(fdusb, 0) < 0)
	{
		if(verbose)
			fprintf(stderr, "< pid=%d > pusb_claim_interface 1 failed\n",
					this_process);
		return(-1);
	}

	ep_int = pusb_endpoint_open(fdusb, EP_INT, O_RDWR);
	if(ep_int == NULL)
	{
		if(verbose) fprintf(stderr, "< pid=%d > pusb_endpoint_open failed\n",
							this_process);
		return -1;
	}

	ep_data_in = pusb_endpoint_open(fdusb, EP_DATA_IN, O_RDWR);
	if(ep_data_in == NULL)
	{
		if(verbose) fprintf(stderr, "< pid=%d > pusb_endpoint_open failed\n",
							this_process);
		return -1;
	}

	/*
	 * endpoint 0x02 is used for sending ATM cells, this function
	 * is an infinite loop and will return only on errors.
	 */
	
	ep_data_out = pusb_endpoint_open(fdusb, EP_DATA_OUT, O_RDWR);
	if(ep_data_out == NULL) 
	{
		if(verbose)
			fprintf(stderr, "< pid=%d > pusb_endpoint_open failed\n",
					this_process);
		return -1;
	}

	init_ep_int(ep_int);
	init_ep_data_in(ep_data_in);

	handle_endpoint_02(ep_data_out, fdin,ep_data_in,ep_int);

	/* we release all the interface we'd claim before exiting */
	if(pusb_release_interface(fdusb, 0) < 0 && verbose)
		fprintf(stderr, "< pid=%d > pusb_release_interface failed\n",
				this_process);

	pusb_endpoint_close(ep_int);
	pusb_endpoint_close(ep_data_in);
	pusb_endpoint_close(ep_data_out);

	pusb_close(fdusb);

	return(0);

}
