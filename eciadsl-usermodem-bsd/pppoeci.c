/*
  Author : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 29/11/2001

*********************************************************************
 File		: 	$RCSfile$
 Version	:	$Revision$
 Modified by	:	$Author$ ($Date$)
 Licence	:	GPL
*********************************************************************

Device: VendorID=0x0915 ProductID=0x0204 Class=ff/ff/ff, 1 configuration(s)
Manufacturer: GlobeSpan Inc. Product: USB-ADSL Modem SN: 040506
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

	26/12/2001 : Rewrite the select() and ioctl() loop. We will have 2 threads
	thread 1: for (;;) { ppp_read() ; aal5_write(); }
	thread 2: init_ep_int, init_ep_data, for (;;) { BIG_ioctl(REAPURB);
	  if (urb == urb_int) handle_ep_int().
	  if (urb == urb_data) handle_ep_data().

        5/7/2002: Added LLC encapsulation support for users from switzerland (rfc2364)
                  Added LLC+SNAP encapsulation support (Flynux's patch, rfc1483)
                  to add a new encapsulation method:
                     - add its name to enum encapsulation_modes and array mode_name
                     - add the frame header to array frame_header
                     - specify the frame header length in array frame_header_len 
                     - update max_frame_header_len if necessary 
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>      /* N_HDLC & TIOCSETD */
#include <sys/time.h>
#include <sys/resource.h>	/* setpriority() */
#include <string.h>
#include <sched.h>		/* for sched_setscheduler */
#include <limits.h>		/* for LONG_MAX */
#include <sys/types.h>
#include <sys/socket.h>
#if defined(__linux__)
#include <linux/if_tun.h>
#include <linux/if.h>
#endif

#include "util.h"
/* my USB library */
#include "pusb.h"
/* GS 7070 control routines */
#include "gs7070.h"

#define LOG_FILE "/tmp/pppoeci.log"

#define PPP_BUF_SIZE 64*1024

char* exec_filename;

extern struct config_t config;

typedef enum
{
	/* must be the first element since (frame_type==0)
	   must mean no encapsulation */
	VCM_RFC2364 = 0,
	LLC_RFC2364,
	LLC_SNAP_RFC1483_BRIDGED_ETH_NO_FCS,
	VCM_RFC_1483_BRIDGED_ETH,
	LLC_RFC1483_ROUTED_IP,
	VCM_RFC1483_ROUTED_IP, 
	modes_count
} encapsulation_mode;

/* defaults to no mode selected, we'll set the mode later
   based on config file or commandline */
int frame_type=-1;

const char* mode_name[] =
{
	"VCM_RFC2364",
	"LLC_RFC2364",
	"LLC_SNAP_RFC1483_BRIDGED_ETH_NO_FCS",
	"VCM_RFC_1483_BRIDGED_ETH",
	"LLC_RFC1483_ROUTED_IP",
	"VCM_RFC1483_ROUTED_IP"
};

encapsulation_mode default_frame_type=VCM_RFC2364;

#ifdef USE_BIG_ENDIAN
#define HDLC_HEADER (short)0xff03
#else
#define HDLC_HEADER (short)0x03ff
#endif

const char* frame_header[] =
{
	"",											/* VCM_RFC2364 */
	"\xfe\xfe\x03\xcf",							/* LLC_RFC2364 */
	"\xaa\xaa\x03\x00\x80\xc2\x00\x07\x00\x00", /* LLC_SNAP_RFC1483_BRIDGED_ETH_NO_FCS */
	"\x00\x00", 								/* VCM_RFC_1483_BRIDGED_ETH */
	"\xaa\xaa\x03\x00\x00\x00\x08\x00",			/* LLC_RFC1483_ROUTED_IP */
	""											/* VCM_RFC1483_ROUTED_IP */
};

#define max_frame_header_len 10
const size_t frame_header_len[] =
{
	0,	/* VCM_RFC2364 */
	4,	/* LLC_RFC2364 */
	10,	/* LLC_SNAP_RFC1483_BRIDGED_ETH_NO_FCS */
	2,	/* VCM_RFC_1483_BRIDGED_ETH */
	8,	/* LLC_RFC1483_ROUTED_IP */
	0 	/* VCM_RFC1483_ROUTED_IP */
};

/* #define CHECK_FRAME_HEADER 1  // just overload i suppose.. */

/* main program error codes (must be returned as negative ints) */
typedef enum
{
	ERR_NONE = 0,
	ERR_USAGE,
	ERR_SIGTERM,
	ERR_OPEN_LOGFILE,
	ERR_LOADING_N_HDLC,
	ERR_FOUNDING_MODEM,
	ERR_PUSB_SET_INTERFACE,
	ERR_PUSB_CLAIM_INTERFACE,
	ERR_PUSB_OPEN_EP_INT,
	ERR_PUSB_OPEN_EP_DATA_IN,
	ERR_PUSB_OPEN_EP_DATA_OUT,
	ERR_PUSB_INIT_EP_INT,
	ERR_PUSB_INIT_EP_DATA_INT,
	ERR_WRONG_FRAME_HEADER,
	ERR_TUN,
	ERR_FORK
} error_codes;

#define ERR_BUFSIZE (1024 - 1)
char errText[ERR_BUFSIZE + 1];

#define PAD_SIZE 11
/* for debugging only */
void dump_urb(char* buffer, int size);

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

/* USB level */
#define DATA_TIMEOUT 300
int data_timeout = 0;

#include "modem.h"

/* max size of data endpoint (Windows driver uses 448). */
#define MAX_EP_SIZE 448
/* number of simultaneous URB submitted to data endpoint (Windows drv : 20 */
#define NB_PKT_EP_DATA_IN 20
/* number of ISO packet per URB (Windows driver uses 10) */
#define PKT_NB 10

/* ATM level : adapt according your ADSL provider settings */
unsigned int my_vpi = 0xffffffff;
unsigned int my_vci = 0xffffffff;

unsigned int vendor = 0;
unsigned int product = 0;

int pusb_set_interface_alt = 0;

#define CELL_SIZE 53
#define CELL_HDR  05
#define CELL_DATA 48

int syncHDLC = 1;

#define synclen 2

/* global parameters */
int verbose = 0;
int usb_sync = 1;
pusb_device_t fdusb;
/* unsigned char lbuf[20][4400]; */

/*
  ep_data_out : 0x02 bulk
  ep_data_in  : 0x88 isoc
  ep_int      : 0x86 intr
*/

pusb_endpoint_t g_ep_data_out, g_ep_data_in, g_ep_int;
pid_t this_process;	 /* always the current process pid */

/* predeclarations */

int aal5_read(unsigned char* cell_buf, /* size_t count, */
				unsigned int vpi, unsigned int vci,
				unsigned int pti, int fdout);

/*
 * variable that will be used in several function
 *   => so global
 */

int g_fdout;
int g_fdin;

#define CRC32_REMAINDER CBF43926
#define CRC32_INITIAL 0xffffffff

#define CRC32(c, crc) (crc32tab[((size_t)((crc) >> 24) ^ (c)) & 0xff] ^ (((crc) << 8)))

unsigned long crc32tab[256] =
{
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

unsigned int calc_crc(unsigned char* mem, int len, unsigned int initial)
{
	unsigned int crc;

	crc = initial;

	for (; len; mem++, len--)
		crc = CRC32(*mem, crc);

	return(crc);
}

/*
 * display a convenient string about the current time.
 * the line printed is not \n terminated
 */

void print_time(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	printf("< pid=%u > [%.24s %6ld] ", getpid(), ctime(&tv.tv_sec), tv.tv_usec);
}

void message(const char* text)
{
	print_time();
	printf("%s\n", text);
}

void fatal_error(const error_codes err, const char* text)
{
	if (text && verbose)
		message(text);
	exit(-err);
}

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

/*
 * ppp_read returns a pointer to a static buffer.
 * it returns the number of bytes read or -1 in case of errors.
 * 0 should indicates that pppd exited (not tested).
 */

int ppp_read(int fd, unsigned char** buf, int* n)
{
	/*
	 * we must handle HDLC framing, since this is what pppd
	 * sends to us. I use some code from rp-pppoe
	 */

	static unsigned char frame_buf[max_frame_header_len + PPP_BUF_SIZE];
	static unsigned char* ppp_buf;

	int r;

        if (frame_type)
		{
            if (syncHDLC)
                ppp_buf = frame_buf+frame_header_len[frame_type]-synclen;
            else
                ppp_buf = frame_buf+frame_header_len[frame_type];
        }
        else
			ppp_buf = frame_buf;

	for (;;)
	{

		do
		{
			r = read(fd, ppp_buf, PPP_BUF_SIZE);
		}
		while (r < 0 && errno == EINTR);

		if (r < 0)
		{
			message("error reading from PPP");
			perror("reason");
		}

		/* debug message */
		if (verbose > 1)
		{
			snprintf(errText, ERR_BUFSIZE, "reading from PPP: len=%d", r);
			message(errText);
			dump(ppp_buf, r);
		}

		if (r <= 0)
			return(r);

		/* suppress leading 2 bytes */

		if (syncHDLC)
		{
			if (r < (synclen+1))
			{
				snprintf(errText, ERR_BUFSIZE,
						"reading from PPP: short on data, r=%d => ignored", r);
				message(errText);
				continue;
			}

			if (r == PPP_BUF_SIZE)
			{
				snprintf(errText, ERR_BUFSIZE,
						"reading from PPP: too much data, r=%d => ignored", r);
				message(errText);
				continue;
			}

			if (frame_type)
			{
				unsigned char* s;
				int i;
				int l;

				l = frame_header_len[frame_type];
				s = (unsigned char*)frame_header[frame_type];
				for (i = 0; l--; ++i)
					frame_buf[i] = s[i];
				*buf = frame_buf;
				*n = r - synclen + frame_header_len[frame_type];
			}
			else
			{
				*buf = frame_buf + synclen;
				*n = r - synclen;
			}
			return(*n);
		}
		else /* no syncHDLC for pppoe */
		{
			unsigned char* s;
			int i;
			int l;
			
			if (r < 1)
			{
				snprintf(errText, ERR_BUFSIZE,
						"reading from PPP: short on data, r=%d => ignored", r);
				message(errText);
				continue;
			}

			if (r == PPP_BUF_SIZE)
			{
				snprintf(errText, ERR_BUFSIZE,
						"reading from PPP: too much data, r=%d => ignored", r);
				message(errText);
				continue;
			}

			l = frame_header_len[frame_type];
			s = (unsigned char*)frame_header[frame_type];
			for (i = 0; l--; ++i)
				frame_buf[i] = s[i];
			*buf = frame_buf;
			*n = r + frame_header_len[frame_type];

			return(*n);
		}
		break;
	}
}

#define NOBUF_RETRIES 5

int ppp_write(int fd, unsigned char* buf, int n)
{
	/* static unsigned char ppp_buf[synclen+max_frame_header_len+PPP_BUF_SIZE]; */
	static int errs = 0;

	int r;
	int retries;

	if (syncHDLC)
	{
		static unsigned char ppp_buf[synclen+max_frame_header_len + PPP_BUF_SIZE];
		*(short*)ppp_buf = HDLC_HEADER;

		if (frame_type)
		{
#ifdef CHECK_FRAME_HEADER 
			int l;
			int i = 0;

			for (l = frame_header_len[frame_type]; l--; ++i)
			{
				if (((unsigned char*)frame_header[frame_type])[i] != ((unsigned char*)buf)[i])
				{
					if (verbose)
					{
						snprintf(errText, ERR_WRONG_FRAME_HEADER,
								"wrong frame header:");
								message(errText);
						dump(buf, n);
					}
					return(0); /* drop the packet */
				}
			}
#endif
			n -= frame_header_len[frame_type];
			memcpy(ppp_buf + synclen, buf + frame_header_len[frame_type], n);
		}
		else
			memcpy(ppp_buf + synclen, buf, n);
                
		if (verbose > 1)
		{
			snprintf(errText, ERR_BUFSIZE,
					"writing to PPP: len=%d", n + 2);
			message(errText);
			dump(ppp_buf, n + 2);
		}

		retries = 0;
		while (retries < NOBUF_RETRIES)
		{
			if ((r = write(fd, ppp_buf, n + 2)) < 0)
			{
				/*
				 * We sometimes run out of mbufs doing simultaneous
				 * up- and down-loads on FreeBSD.  We should find out
				 *  why this is, but for now...
				 */

				if (errno == ENOBUFS)
				{
					errs++;

					if (verbose && (errs < 10 || errs % 100 == 0))
					{
						snprintf(errText, ERR_BUFSIZE,
								"writing to PPP: %d ENOBUFS errors", errs);
						message(errText);
					}

					if (retries++ < NOBUF_RETRIES)
						/* retry after delay */
						sleep(1);
				}
				else
				{
					message("error writing to PPP");
					perror("reason");
					/* unexpected error */
					return(r);
				}
			}
			else
				/* PPP write is OK */
				return(r);
		}
		/* all retries have failed */
		/* throw away the packet */
		return(0);
	}
	else /* no syncHDLC for pppoe */ 
	{

	    static unsigned char ppp_buf[max_frame_header_len + PPP_BUF_SIZE];

#ifdef CHECK_FRAME_HEADER 
			int l;
			int i = 0;

			for (l = frame_header_len[frame_type]; l--; ++i)
			{
				if (((unsigned char*)frame_header[frame_type])[i] != ((unsigned char*)buf)[i])
				{
					if (verbose)
					{
						snprintf(errText, ERR_WRONG_FRAME_HEADER,
								"wrong frame header:");
								message(errText);
						dump(buf, n);
					}
					return(0); /* drop the packet */
				}
			}
#endif
			n -= frame_header_len[frame_type];
			memcpy(ppp_buf, buf + frame_header_len[frame_type], n);

                
		if (verbose > 1)
		{
			snprintf(errText, ERR_BUFSIZE,
					"writing to PPP: len=%d", n + 2);
			message(errText);
			dump(ppp_buf, n + 2);
		}

		retries = 0;
		while (retries < NOBUF_RETRIES)
		{
			if ((r = write(fd, ppp_buf, n)) < 0)
			{
				/*
				 * We sometimes run out of mbufs doing simultaneous
				 * up- and down-loads on FreeBSD.  We should find out
				 *  why this is, but for now...
				 */

				if (errno == ENOBUFS)
				{
					errs++;

					if (verbose && (errs < 10 || errs % 100 == 0))
					{
						snprintf(errText, ERR_BUFSIZE,
								"writing to PPP: %d ENOBUFS errors", errs);
						message(errText);
					}

					if (retries++ < NOBUF_RETRIES)
						/* retry after delay */
						sleep(1);
				}
				else
				{
					message("error writing to PPP");
					perror("reason");
					/* unexpected error */
					return(r);
				}
			}
			else
				/* PPP write is OK */
				return(r);
		}
		/* all retries have failed */
		/* throw away the packet */
		return(0);
	}

	return(-1);
}

/*
 * cell_read : is used to decode an individual cell and to try to make
 * an AAL5 frame.
 */

int cell_read(unsigned char* cellbuf, /* size_t count, */ int fdout)
{
	unsigned int vpi, vci, pti;

	/* decoding & printing of the cell */
	vpi = ( cellbuf[0]         <<  4) | (cellbuf[1] >> 4);
	vci = ((cellbuf[1] & 0x0f) << 12) | (cellbuf[2] << 4) | (cellbuf[3] >> 4);
	pti = ( cellbuf[3] & 0x0f);

	if (vpi != my_vpi || vci != my_vci)
	{
		/* there's some junk so try to re-sync */
		return(-2);
	}
	/* filling our AAL5 frame */
	return(aal5_read(cellbuf + CELL_HDR, /* CELL_DATA, */ vpi, vci, pti, fdout));
}

int cell_write(unsigned char* cellbuf, unsigned char* buf, int n,
			 unsigned int vpi, unsigned int vci, unsigned int pti)
{
	unsigned char gfc = 0;
	unsigned char hec = 0x00; /* change to the value used in the Windows driver */

	if (n != CELL_DATA)
		return(-1);

	/* normalize */
	vpi &= 0x0fff;
	vci &= 0xffff;
	pti &= 0x0f;

	cellbuf[0] = (gfc << 4) | (vpi >> 4);
	cellbuf[1] = (vpi << 4) | (vci >> 12);
	cellbuf[2] = (vci >> 4);
	cellbuf[3] = ((vci & 0xf) << 4) | pti;
	cellbuf[4] = hec;

	memcpy(cellbuf + CELL_HDR,  buf, CELL_DATA);
	memset(cellbuf + CELL_SIZE, 0xff, PAD_SIZE);

	return(CELL_SIZE + PAD_SIZE);
}

/*
 * parse an ATM cell and try to complete (or make a new one)
 * AAL5 frame. If success, the frame is sent to pppd. If error, -1 is returned
 * (0 otherwise)
 */

int aal5_read(unsigned char* cell_buf, /* size_t count, */
		unsigned int vpi, unsigned int vci, unsigned int pti, int fdout)
{
	static unsigned char buf[64 * 1024];
	static int len = 0;

	int ret;
	unsigned int real_len;
	unsigned int crc1, crc2;

	if (vpi != my_vpi || vci != my_vci)
	{
		if (verbose > 1)
		{
			snprintf(errText, ERR_BUFSIZE,
					"vpi.vci mismatch (%d.%d instead of %d.%d) => ignored",
					 vpi, vci, my_vpi, my_vci);
			message(errText);
			dump(cell_buf, CELL_DATA);
		}
		return(-2); /* return -2 and ? */
	}

	/*
	 * We should add a check for OAM cells, which should be done before
	 * copying the cell's content to our buffer
	 */

	if ((unsigned int)(len + CELL_DATA) < sizeof(buf))
	{
		memcpy(buf + len, cell_buf, CELL_DATA);
		len += CELL_DATA;
	}
	else
	{
		if (verbose)
		{
			snprintf(errText, ERR_BUFSIZE,
					"aal5_read: buffer overflow (len=%d) => resetting buffer", len);
			message(errText);
		}
		len = 0;
		return(0);
	}

	/* check if PTI=1, which indicated last cell of an AAL5 frame */
	if ((pti & 2) == 0)
		return(0);

	real_len = (buf[len - 6] << 8) | buf[len - 5];

	if (verbose > 1)
	{
		snprintf(errText, ERR_BUFSIZE,
				"aal5_read: len=%d => real_len=%d", len, real_len);
		message(errText);
	}

	/*
	  Check the CRC of received AAL5 frames.
	  crc1 : computed crc on the current AAL5 frame (the theoretical value)
	  crc2 : crc read from the AAL5 frame (the real value)
	*/

	crc1 = ~calc_crc(buf, len - 4, -1);
	crc2 = (buf[len - 4] << 24) | (buf[len - 3] << 16)
			| (buf[len - 2] << 8) | buf[len - 1];

	if (crc1 != crc2 && !(verbose > 1))
	{
		snprintf(errText, ERR_BUFSIZE,
				"aal5_read: len=%d => real_len=%d", len, real_len);
		message(errText);
	}

	if (crc1 != crc2)
	{
		snprintf(errText, ERR_BUFSIZE,
				"bad crc %08x (instead of %08x) => dropping AAL5 frame",
				crc2, crc1);
		message(errText);
		len = 0;
		return(0);
	}

	ret = ppp_write(fdout, buf, real_len);
	len = 0;

	/*
	  We handle error here : this is a hack!
	*/

	if (ret < 0)
	{
		snprintf(errText, ERR_BUFSIZE,
				"error writing to PPP: %d => leaving", ret);
		fatal_error(ERR_NONE, errText);
	}

	return(ret);
}

/*
 * Warning: (ok, tooks me half an hour to find my own buf)
 * buf must be large enough to contains the whole AAL5 frame,
 * with its layer.
 */

int aal5_write(pusb_endpoint_t epdata, unsigned char* buf, int n)
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
	unsigned char* bigbuf = (unsigned char*)malloc(1367 * 53);
	if (bigbuf == NULL)
		return(-1);
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
	while (total_len > 0)
	{

		if (total_len <= CELL_DATA)
			/* this is the last cell, so mark it */
			pti = 2;

		ptr += cell_write(bigbuf + ptr, buf,
				CELL_DATA, my_vpi, my_vci, pti);

		/* send the same cell over another vpi/vci */

		buf += CELL_DATA;
		total_len -= CELL_DATA;

	}

	/* debug message */
	if (verbose > 1)
	{
		snprintf(errText, ERR_BUFSIZE,
				"writing to USB: len=%d", ptr);
		message(errText);
		dump(bigbuf, ptr);
	}
/*
	{
		static int count = 0, count_seconds = 0;
		static time_t last_time = 0;
		time_t now;

		count += ptr;
		count_seconds += ptr;

		print_time();
		time(&now);
		if (now != last_time)
		{
			printf("Writing %d bytes to USB, last second = %d bytes\n", ptr,
				   count_seconds);

			count_seconds = 0;
			last_time = now;
		}
		else
			printf("Writing %d bytes to USB\n", ptr);
	}
*/
	ret = pusb_endpoint_iso_write(epdata, bigbuf, ptr);
/*
	ret = pusb_endpoint_submit_write (epdata, bigbuf, ptr, 0);
*/
	if (ret < 0)
	{
		message("error writing to USB");
		perror("reason");
	}

	return(ret);
}
/* returns -1 in case of errors */

int decode_usb_pkt(unsigned char* buf, int len)
{
	static unsigned char rbuf [64 * 1024];
	static int rlen = 0;

	int r;
	int pos;

	/* debug message */
	if (verbose > 1)
	{
		snprintf(errText, ERR_BUFSIZE,
				"reading from USB: len=%d", len);
		message(errText);
		dump(buf, len);
	}

	/* add the received buf to our buf, if possible */

	if ((unsigned int)(rlen + len) < sizeof(rbuf))
	{
		memcpy(rbuf + rlen, buf, len);
		rlen += len;
	}
	else
	{
		snprintf(errText, ERR_BUFSIZE,
				"warning: no space for %d bytes", len);
		message(errText);
	}

	pos = 0;
	while ((rlen - pos) >= CELL_SIZE)
	{
		if ((r = cell_read(rbuf + pos, g_fdout)) < 0 && r != -2)
		{
			rlen -= pos;
			if (rlen != 0)
				memcpy(rbuf, rbuf + pos, rlen);
			return(r);
		}
		if (r == -2)
			/* we got out of sync or there was some junk so try to find the next
			block header (just add 1 onto pos!!) for now!! */
			pos++;
		else
			pos += CELL_SIZE;
	}
	rlen -= pos;

	if (rlen != 0)
		memcpy(rbuf, rbuf + pos, rlen);

	return(0);
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

	int i, ret;

	for (i = 0; i < NB_PKT_EP_INT; i++)
	{
		/* we use a BULK transfer instead, since INTERRUPT transfer
		   are not supported by usbdevfs (devio.c)
		*/
		ret = pusb_endpoint_int_read(ep_int, buf[i],sizeof(buf[i]));
		if (ret < 0)
		{
			snprintf(errText, ERR_BUFSIZE,
					"error on initial submit of URB %d on EP_INT", i);
			message(errText);
			perror("error: init_ep_int");
			return(ret);
		}
	}

	return(0);
}

int init_ep_data_in(pusb_endpoint_t ep_data_in)
{
	static unsigned char buf[NB_PKT_EP_DATA_IN][MAX_EP_SIZE * PKT_NB];

	int i, ret;

	for (i = 0; i < NB_PKT_EP_DATA_IN; i++)
	{
		ret = pusb_endpoint_iso_read(ep_data_in, buf[i], MAX_EP_SIZE);
		if (ret < 0)
		{
			snprintf(errText, ERR_BUFSIZE,
					"error on initial submit of URB %d on EP_DATA_IN", i);
			message(errText);
			perror("error: init_ep_data_in");
			return(ret);
		}
	}

	return(0);
}

void handle_ep_int(unsigned char* buf, int size, pusb_device_t fdusb)
{
	static int lost_synchro = 0;
	static GS7070int* gs = 0;

	unsigned char outbuf[40] =
	{
		0xff, 0xff, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c,
		0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c,
		0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c,
		0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  0x0c,
		0x0c, 0x0c
	};
	int i, outi = 0;
	int dataBlock = 0;

	if (gs==0)
		gs=allocateGS7070int();

	if (verbose > 1)
	{
		snprintf(errText, ERR_BUFSIZE,
				"reading from ep int: len=%d", size);
		message(errText);
		dump(buf, size);
	}

	if (!lost_synchro || 1)
	{
		for (i = 3; i < 18; i++)
		{
			unsigned short code = (buf[2 * i + 0] << 8) | buf[2 * i + 1];

			if (code != 0x0c0c)
			{ /* don't bother with 0c0c blocks, there 'empty' */
				dataBlock=-1;
				gs7070SetControl(gs, buf + i * 2);
			}

			/* this code need to be put into gs7070.c
			if (w != 0x0c0c && w != 0x734d && w != 0x7311 && w != 0xf301
				&& w != 0xf34f && w!= 0xf343 && w!= 0x5311 && w != 0xf313
				&& w != 0x7341)
			{
				snprintf(errText, ERR_BUFSIZE,
						"synchro loss! w=%04x", w);
				message(errText);
				lost_synchro = 1;
			}*/
		}
	}
	if (dataBlock == 0)
		/* no data/control information so don't bother to respond */
		return;

	for (i = 0; i < 15; i++)
	{
		unsigned char b1, b2;

		b1 = buf[6 + 2 * i + 0];
		b2 = buf[6 + 2 * i + 1];

		/* each word != 0x0c 0x0c is copied in the output buffer */

		if (b1 != 0x0c || b2 != 0x0c)
		{
			if ((unsigned int)(10 + outi) >= sizeof(outbuf))
			{
				message("warning: outbuf is too small");
				dump(buf, size);
				break;
			}
			outbuf[10 + outi] = b1;
			outbuf[10 + outi+1] = b2;
			gs7070GetResponse(gs, outbuf + 10 + outi);
			outi += 2;
		}
	}

	if (pusb_control_msg(fdusb, 0x40, 0xdd, 0xc02, 0x580, outbuf,
			sizeof(outbuf)) != sizeof(outbuf))
	{
		message("error on sending vendor device URB");
		perror("error: can't send vendor device!");
	}
	else
	{
		if (verbose > 1)
		{
			snprintf(errText, ERR_BUFSIZE,
			"ctrl msg sent: len=%d", sizeof(outbuf));
			message(errText);
			dump(outbuf, sizeof(outbuf));
		}
	}
}

void handle_ep_data_in(unsigned char* buf, int size)
{
	decode_usb_pkt(buf, size);
}

/*
   endpoint 0x02 is used for sending ATM cells

   to handle synchronous write, we does an infinite
   loop, keeping writing on endpoint EP_DATA_OUT
*/

void handle_ep_data_out(pusb_endpoint_t epdata, int fdin)
{
	int r, n, tcount;
	unsigned char* buf;
	tcount = 0;
	
	for (;;)
	{
	    n = ppp_read(fdin, &buf, &n);

	    if (n <= 0)
		{
			snprintf(errText, ERR_BUFSIZE,
					"reading from PPP=%d", n);
			message(errText);
			break;
		}

	    r = aal5_write(epdata, buf, n);

	    if (r < 0)
		{
			snprintf(errText, ERR_BUFSIZE,
					"aal5_write=%d", r);
			message(errText);
			tcount++;
			if (tcount > 3)
			    break;
		}
	    else
			if (tcount > 0)
				tcount--;
	}

	snprintf(errText, ERR_BUFSIZE,
			"end of handle_ep_data_out tcount=%d", tcount);
	message(errText);
}

/*
  thread_ep_data_out: reads data from PPP (g_fdin) and write them to
  USB (g_ep_data_out)
*/

void* thread_ep_data_out(void* ignored)
{
	handle_ep_data_out(g_ep_data_out, g_fdin);

	/* when this thread is terminating, we need to exit the whole process */
	exit(0);

    /* this tricks C compiler, return() is never matched, and void*
       ignored is now used */
	return ignored;
}

/*
  thread_ep_data_in: reads data from USB (g_ep_data_in) and write them
  to PPP (g_fdout)
*/

void * thread_ep_data_in(void * ignored)
{
    for (;;)
    {
        unsigned char buf [MAX_EP_SIZE];
        int ret;

        ret = pusb_endpoint_iso_read(g_ep_data_in, buf, sizeof(buf));
        if (ret < 0)
        {
            message("error on re-submit URB on EP_DATA_IN");
            perror("error: can't re-submit on ep EP_DATA_IN");
        }
        else
        {
            printf("ep_data_in : read %d byte(s)\n",ret);
            handle_ep_data_in(buf, ret);
        }
    }

    return ignored;
}

void * thread_ep_int(void * ignored)
{
    for (;;)
    {
        unsigned char buf [0x40];
        int ret;
        
        ret = pusb_endpoint_int_read(g_ep_int, buf, sizeof(buf));
        if (ret < 0)
        {
            message("error on re-submit URB on EP_INT");
            perror("error: can't re-submit on EP_INT");
        }
        else
        {
            /* printf("ep_int : read %d byte(s)\n",ret); */
            handle_ep_int(buf, ret, fdusb);
        }
    }

    return ignored;
}

void version(const int full)
{
	printf("%s, part of " PRODUCT_NAME " (" PRODUCT_ID ") " PRODUCT_VERSION " (" __DATE__ " " __TIME__ ")\n",
			exec_filename);
	if (full)
		printf("%s\n", id);
	_exit(ERR_NONE);
}


void display_modes(void)
{
	encapsulation_mode i;
	for (i = 0; i < modes_count; i++)
		fprintf(stderr, "%s%s\n", mode_name[i], (i == default_frame_type)?" (default)":"");
}

void usage(const int ret)
/* if 'error' is 0, usage() hasn't to quit with an <0 error code */
{

	fprintf(stderr,	"usage:\n"
					"       %s [<switch>..] [-vpi num -vci num -vendor hex -product hex]\n", exec_filename);
	fprintf(stderr,	"switches:\n"
					"       -alt <num>           force the use of an alternate method to set USB interface\n"
					"       -dto <num>           set the DATA_TIMEOUT value (default is %d)\n"
                    "       -mode <name>         PPP encapsulation method (see below)\n"
					"       -v or --verbosity    define the verbosity level [0-2] (enables logging)\n",
					DATA_TIMEOUT);
	fprintf(stderr,	"       -f or --logfile      define the log filename to use (default " LOG_FILE ")\n"
					"       -h or --help         display this message then exit\n"
					"       -V or --version      display the version number then exit\n"
					"       --modes              display a list of the supported modes (to use with -mode)\n"
					"\n");
	fprintf(stderr, "vpi value must be between 0 and %u\n", 0xfff);
	fprintf(stderr, "vci value must be between 0 and %u\n", 0xffff);
	fprintf(stderr, "If ALL parameters but switches are omitted, the ones from " CONF_PATH CONF_DIR "/eciadsl.conf\n"
					"are assumed.\n"
					"\n");
	fprintf(stderr,	"The vpi and vci are numerical values. They define the vpi.vci used\n"
					"by provider. For instance: 8.35 for France, 0.38 for UK.\n"
					"\n");
	fprintf(stderr,	"The vendor and product are hexadecimal values (4-digit with a leading 0x)\n"
					"They define the vendor ID and product ID. For instance: -vendor 0x0915\n"
					"-product 0x8000 for an ECI Telecom USB ADSL Wan Modem.\n"
					"\n");
	fprintf(stderr,	"The encapsulation mode may differ depending on the country/provider/modem.\n"
					"If you experience LCP requests timeout or IOCTL errors at connect time, try\n"
					"another one (e.g.: LLC_RFC2364 for Switzerland).\n");
	fprintf(stderr, "Possible values:\n");
	display_modes();
	_exit(ret);
}

void sig_term(int sig)
{
	snprintf(errText, ERR_BUFSIZE,
			"SIGTERM (%d) received.. exiting", sig);
	fatal_error(ERR_SIGTERM, errText);
}

#ifdef __linux__
int tap_open(char* dev, int tun)
{
	struct ifreq ifr;
	int fd, err;

	if ((fd = open("/dev/net/tun", O_RDWR | O_SYNC)) < 0)
	return(-1);

	memset(&ifr, 0, sizeof(ifr));
	if (tun)
		ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	else
		ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	if (*dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);

	if ((err = ioctl(fd, TUNSETIFF, (void*) &ifr)) < 0)
	{
		close(fd);
		return(err);
	}
	strcpy(dev, ifr.ifr_name);
	return(fd);
}
#endif

int set_mode(const char* mode)
{
	int j;

	j = -1;
	while (++j < modes_count)
		if (!strcasecmp(mode, mode_name[j]))
		{
			frame_type = j;
			return(1);
		}
	return(0);
}

int main(int argc, char** argv)
{
	const char* logfile = LOG_FILE;
	int fdin, fdout, log;
	int i;
	char dev[20];

	*dev=0;
	exec_filename=basename(*argv);
	this_process = getpid();
	log = 0;

	/* parse command line options */
	for (i = 1; i < argc; i++)
	{
		if (((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbosity") == 0) )
			&& (i + 1 < argc))
			get_signed_value(argv[++i], &verbose);
		else
		if ((strcmp(argv[i], "-vpi") == 0) && (i + 1 < argc))
			get_unsigned_value(argv[++i], &my_vpi);
		else
		if ((strcmp(argv[i], "-vci") == 0) && (i + 1 < argc))
			get_unsigned_value(argv[++i], &my_vci);
		else
		if ((strcmp(argv[i], "-vendor") == 0) && (i + 1 < argc))
			get_hexa_value(argv[++i], &vendor);
		else
		if ((strcmp(argv[i], "-product") == 0 )&& (i + 1 < argc))
			get_hexa_value(argv[++i], &product);
		else
		if ((strcmp(argv[i], "-alt") == 0) && (i + 1 < argc))
			get_signed_value(argv[++i], &pusb_set_interface_alt);
		else
		if ((strcmp(argv[i], "-dto") == 0) && (i + 1 < argc))
			get_signed_value(argv[++i], &data_timeout);
		else
		if ((strcmp(argv[i], "--version") == 0) || (strcmp(argv[i], "-V") == 0))
			version(0);
		else
		if (strcmp(argv[i], "--modes") == 0)
		{
			display_modes();
			_exit(0);
		}
		else
		if (strcmp(argv[i], "--full-version") == 0)
			version(1);
		else
		if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0))
			usage(ERR_NONE);
		else
		if (((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--logfile") == 0) )
			&& (i + 1 < argc))
		{
			logfile = argv[++i];
			if (!verbose)
				verbose = 1;
		}
		else
			if ((strcmp(argv[i], "-mode") == 0) && (i + 1 < argc))
			{
				if (!set_mode(argv[++i]))
					usage(-1);
			}
			else
				usage(-1);
	}

	/* read the configuration file */
	read_config_file();

	/* try to assume default values from the config file */
	if ((my_vci == 0xffffffff) && (config.vci != 0xffffffff))
			my_vci = config.vci;
	if ((my_vpi == 0xffffffff) && (config.vpi != 0xffffffff))
			my_vpi = config.vpi;
	if (!vendor && (config.vid2))
			vendor = config.vid2;
	if (!product && (config.pid2))
			product = config.pid2;

	if (my_vci == 0xffffffff || my_vpi == 0xffffffff
		|| !vendor || !product)
	{
		fprintf(stderr, "vci, vpi, vendor or product ID couldn't be guessed:\n");
		fprintf(stderr, "no default parameters found in config file, couldn't assume default values\n");
		usage(-1);
	}	
	/* now set the mode from .conf file if not specified on
	   command-line else we default to VCM_RFC2364
	*/
	if (frame_type == -1) 
	{
		if (config.mode)
		{
			if (!set_mode(config.mode))
			{
				fprintf(stderr, "bad PPP mode parameter found in config file (%s), couldn't assume default value\n",
					config.mode);
				usage(-1);
			}
		}
		else
			frame_type = default_frame_type;
	}

	/* rfc-3292-56 range values check for vpi and vci */
	if (my_vpi>0xfff)
	{
		fprintf(stderr, "incorrect vpi value (%u overflows)\n", my_vpi);
		usage(-1);
	}
	if (my_vci>0xffff)
	{
		fprintf(stderr, "incorrect vci value (%u overflows)\n", my_vci);
		usage(-1);
	}

	/* check some other params */
	if (pusb_set_interface_alt < 0)
	{
		fprintf(stderr, "incorrect alt interface value (%d underflows)\n",
			pusb_set_interface_alt);
		usage(-1);
	}
	if (data_timeout < 0)
	{
		fprintf(stderr, "incorrect data timeout value (%d underflows)\n",
			data_timeout);
		usage(-1);
	}
	if ((verbose < 0) || (verbose > 2))
	{
		fprintf(stderr, "incorrect verbose level (%d is out of boundaries)\n",
			verbose);
		usage(-1);
	}
	if (vendor > 0xffff)
	{
		fprintf(stderr, "incorrect vendor ID (0x%x overflows)\n", vendor);
		usage(-1);
	}
	if (product > 0xffff)
	{
		fprintf(stderr, "incorrect product ID (0x%x overflows)\n", product);
		usage(-1);
	}

	if (verbose)
	{
/*
		fprintf(stderr, "Using:\n"
						" vpi.vci=%d.%d\n"
						" vendor/product=0x%04x/0x%04x\n"
						" verbose level=%d\n"
						" log file=%s (%sabled)\n",
						my_vpi, my_vci, vendor, product,
						verbose, logfile,
						verbose?"en":"dis");
		if (pusb_set_interface_alt)
			fprintf(stderr, " pusb_set_interface_alt=%d\n", pusb_set_interface_alt);
		if (data_timeout)
			fprintf(stderr, " data timeout=%d\n", data_timeout);
*/
	}
	if (!data_timeout)
		data_timeout=DATA_TIMEOUT;
	
	if (frame_type > VCM_RFC2364)
		syncHDLC = 0;

	/* duplicate in and out fd */
#if defined(__linux__)
	if ((frame_type == VCM_RFC2364) || (frame_type == LLC_RFC2364)) 
	{
#endif
	    fdin  = dup(0);
	    fdout = dup(1);
#if defined(__linux__)
	}
	else
		if ((frame_type == LLC_SNAP_RFC1483_BRIDGED_ETH_NO_FCS)
			|| (frame_type == VCM_RFC_1483_BRIDGED_ETH)) 
			
		{
			    if ((fdin = fdout = tap_open(dev, 0)) == -1 )
				    fatal_error(ERR_TUN, "Could not open tap device");
		}
		else
			if ((frame_type == LLC_RFC1483_ROUTED_IP)
				|| (VCM_RFC1483_ROUTED_IP))
				
			{
				if ((fdin = fdout = tap_open(dev, 1)) == -1 )
					fatal_error(ERR_TUN, "Could not open tun device");
			}
#endif	

	/*
	 * If the verbosity level is greater than 0
	 * we have to create a log
	 */
	if (verbose)
	{
		log = open(logfile, O_CREAT | O_RDWR | O_APPEND, 0600);

		if (log < 0)
		{
			if (verbose)
				perror(logfile);
			fatal_error(ERR_OPEN_LOGFILE, NULL);
		}

		for (i = 0; i < 8; i++)
			write(log, "##########", 10);

		write(log, "\n\n", 2);
	}

	/* Close all standard streams : input, output, error */
	close(0);
	close(1);
	close(2);

		
	/*
	 * Like the verbosity level is > 0
	 * we will redirect all standard streams to the log file
	 * so we are sure to catch all messages from this program :)
	 */
	if (verbose)
	{
		/* duplicate log fd, so 0, 1, 2 point to the log file */
		dup(log);
		dup(log);
		dup(log);

		/* We don't need log fd anymore */
		close(log);

		/* no buffering on stdout & stderr : it doesn't seem to work */
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);

		snprintf(errText, ERR_BUFSIZE,
				"log started (binary compiled on %s %s)", __DATE__, __TIME__);
		message(errText);
	}

	/* debug messages */
	if (verbose > 1)
	{
		message("hi! I'm the parent process, I handle the endpoint 0x07");

		snprintf(errText, ERR_BUFSIZE,
				"file descriptors: fdin=%d, fdout=%d", fdin, fdout);
		message(errText);
	}


	/* Increase priority of the pppoeci process */
	if (setpriority(PRIO_PROCESS, this_process, -20) < 0 && verbose)
	{
		message("setpriority failed");
		perror("reason");
	}

	/*  We fork to background if using an AAL5 extra mode */
	if(frame_type >= LLC_SNAP_RFC1483_BRIDGED_ETH_NO_FCS)
	{
		switch (fork())
		{
			case -1: 
			    fatal_error(ERR_FORK, "fork failed");
			case 0:
			    break;
			default:
			    return(0);
		}
	}

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define SOCKBUF (64 * 1024)
	{
		int sbuf, ss = sizeof(sbuf);

		if (getsockopt(fdin, SOL_SOCKET, SO_SNDBUF, &sbuf, &ss) == 0)
		{

			/* debug message */
			if (verbose > 1)
			{
				snprintf(errText, ERR_BUFSIZE,
						"increasing SNDBUF from %d to %d",
						sbuf, SOCKBUF);
				message(errText);
			}

			sbuf = SOCKBUF;

			if (setsockopt(fdin, SOL_SOCKET, SO_SNDBUF, &sbuf, ss) < 0 && verbose)
			{
				message("setsockopt failed,");
				perror("reason");
			}

		}

		if (getsockopt(fdin, SOL_SOCKET, SO_RCVBUF, &sbuf, &ss) == 0)
		{
			/* debug message */
			if (verbose > 1)
			{
				snprintf(errText, ERR_BUFSIZE,
						"increasing RCVBUF from %d to %d",
						sbuf, SOCKBUF);
				message(errText);
			}

			sbuf = SOCKBUF;

			if (setsockopt(fdin, SOL_SOCKET, SO_RCVBUF, &sbuf, ss) < 0 && verbose)
			{
				snprintf(errText, ERR_BUFSIZE,
						"setsockopt failed",
						this_process);
				message(errText);
				perror("reason");
			}
		}
	}
#endif

	/*
	 * Install HDLC line discipline on fdin if it is a tty and
	 * the OS has such a thing.
	 */

	if (isatty(fdin))
	{
/* #undef N_HDLC */
#ifdef N_HDLC
		int disc = N_HDLC;

		if (ioctl(fdin, TIOCSETD, &disc) < 0)
			fatal_error(ERR_LOADING_N_HDLC, "error loading N_HDLC");
#endif
	}

	/*
	 * Install a SIGTERM signal handler. This is the signal sent by pppd
	 * to kill our process. We do so just to make the log clear on that point
	 */

	signal(SIGTERM, sig_term);

	/*
	 * Install a SIGRTMIN signal handler. This signal is used for the
	 * completion of an URB
	 */

	/* signal(SIGRTMIN, sig_rtmin); */

	/*
	 *  We search for the first USB device matching ST_VENDOR & ST_PRODUCT.
	 *  usbdevfs must be mount on /proc/bus/usb (or you may change the path
	 *  here, according to your config
	 */

	fdusb = pusb_search_open(vendor, product);

	if (fdusb == NULL)
		fatal_error(ERR_FOUNDING_MODEM, "where is this crappy modem?!");

	/* debug message */
	if (verbose > 1)
		message("got the modem, yipiyeah!");

	/* Initialize global variables */
    g_fdin  = fdin;
	g_fdout = fdout;

    pusb_device_set_configuration(fdusb, 1);

	/* set interface */

	if (pusb_set_interface_alt)
		if (pusb_device_set_interface(fdusb, 0, pusb_set_interface_alt) < 0)
		{
			snprintf(errText, ERR_BUFSIZE,
					"pusb_set_interface 0 %d failed", pusb_set_interface_alt);
			fatal_error(ERR_PUSB_SET_INTERFACE, errText);
		}

	/* we claim interface 0, where endpoints 0x02, 0x86 & 0x88 are */
/*
	if (pusb_claim_interface(fdusb, 0) < 0)
		fatal_error(ERR_PUSB_CLAIM_INTERFACE,
					"pusb_claim_interface 0 failed");
*/
	g_ep_int = pusb_endpoint_open(fdusb, EP_INT);
	if (g_ep_int == NULL)
    {
        perror("pusb_endpoint_open (ep_int)");
		fatal_error(ERR_PUSB_OPEN_EP_INT,
					"pusb_endpoint_open EP_INT failed");
    }

	g_ep_data_in = pusb_endpoint_open(fdusb, EP_DATA_IN);
	if (g_ep_data_in == NULL)
    {
        perror("pusb_endpoint_open (ep_data_in)");
		fatal_error(ERR_PUSB_OPEN_EP_DATA_IN,
					"pusb_endpoint_open EP_DATA_IN failed");
    }

	/*
	 * endpoint 0x02 is used for sending ATM cells, this function
	 * is an infinite loop and will return only on errors.
	 */

	g_ep_data_out = pusb_endpoint_open(fdusb, EP_DATA_OUT);
	if (g_ep_data_out == NULL)
    {
        perror("pusb_endpoint_open (ep_data_out)");
		fatal_error(ERR_PUSB_OPEN_EP_DATA_OUT,
					"pusb_endpoint_open EP_DATA_OUT failed");
    }

	/*
	  Relay data between fdin (PPP) => ep_data_out (USB).
	  This is done in a sub process. (or another thread)
	*/

	{
        if (fork() == 0)
        {
            thread_ep_data_out(NULL);
            _exit (0);
        }

        if (fork() == 0)
        {
            thread_ep_data_in(NULL);
            _exit(0);
        }
	}

    thread_ep_int(NULL);

	pusb_endpoint_close(g_ep_int);
	pusb_endpoint_close(g_ep_data_in);
	pusb_endpoint_close(g_ep_data_out);

	pusb_device_close(fdusb);

	return(0);
}
