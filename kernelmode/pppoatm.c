/* pppoatm.c - pppd plugin to implement PPPoATM protocol.
 *
 * Copyright 2000 Mitchell Blank Jr.
 * Based in part on work from Jens Axboe and Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */
#include "pppd.h"
#include "pathnames.h"
#include "fsm.h" /* Needed for lcp.h to include cleanly */
#include "lcp.h"
#include <atm.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <sys/stat.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define _PATH_ATMOPT _ROOT_PATH "/etc/ppp/options"


static struct sockaddr_atmpvc pvcaddr;
static char *qosstr = NULL;
static int pppoatm_accept = 0;
static bool llc_encaps = 0;
static bool vc_encaps = 0;
static int device_got_set = 0;
static int pppoatm_max_mtu, pppoatm_max_mru;

struct channel pppoa_channel;
static char *bad_options[] = {
	"noaccomp", "-ac",
	"default-asyncmap", "-am", "asyncmap", "-as", "escape",
	"receive-all",
	"crtscts", "-crtscts", "nocrtscts",
	"cdtrcts", "nocdtrcts",
	"xonxoff",
	"modem", "local", "sync",
	NULL };

/* returns:
 *  -1 if there's a problem with setting the device
 *   0 if we can't parse "cp" as a valid name of a device
 *   1 if "cp" is a reasonable thing to name a device
 * Note that we don't actually open the device at this point
 * We do need to fill in:
 *   devnam: a string representation of the device
 *   devstat: a stat structure of the device.  In this case
 *     we're not opening a device, so we just make sure
 *     to set up S_ISCHR(devstat.st_mode) != 1, so we
 *     don't get confused that we're on stdin.
 */

static int open_device_pppoatm(void);
static void set_line_discipline_pppoatm(int fd);

static int setdevname_pppoatm(const char *cp)
{
	struct sockaddr_atmpvc addr;
	int fd;
	extern struct stat devstat;
	if (device_got_set)
		return 0;
	info("PPPoATM setdevname_pppoatm");
	memset(&addr, 0, sizeof addr);
	if (text2atm(cp, (struct sockaddr *) &addr, sizeof(addr),
	    T2A_PVC | T2A_NAME) < 0)
		return 0;
	/*if (!dev_set_ok())
		return -1;*/
	memcpy(&pvcaddr, &addr, sizeof pvcaddr);
	strlcpy(devnam, cp, sizeof devnam);
	devstat.st_mode = S_IFSOCK;
	if(the_channel != &pppoa_channel) {
		char **a;

		the_channel = &pppoa_channel;
		info("registering channel\n");
		/**for (a = bad_options; *a != NULL; a++)
			remove_option(*a);*/
	}
	info("PPPoATM setdevname_pppoatm - SUCCESS");
	device_got_set = 1;
	fd = open_device_pppoatm();
	set_line_discipline_pppoatm(fd);
	return 1;
}

static int setspeed_pppoatm(const char *cp)
{
	info("Setspeed pppoatm\n");
	return 0;
}


static void options_for_pppoatm(void)
{  
	info("option for pppoatm\n");
	return;
}


#define pppoatm_overhead() (llc_encaps ? 6 : 2)

static void no_device_given_pppoatm(void)
{
	fatal("No vpi.vci specified");
}


static int open_device_pppoatm(void)
{
	int fd;
	struct atm_qos qos;

	info("open device pppoatm\n");
	if (!device_got_set)
		no_device_given_pppoatm();
	fd = socket(AF_ATMPVC, SOCK_DGRAM, 0);
	if (fd < 0)
		fatal("failed to create socket: %m");
	memset(&qos, 0, sizeof qos);
	qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;

	/* TODO: support simplified QoS setting */

	if (qosstr != NULL)
		if (text2qos(qosstr, &qos, 0))
			fatal("Can't parse QoS: \"%s\"");
	qos.txtp.max_sdu = lcp_allowoptions[0].mru + pppoatm_overhead();
	qos.rxtp.max_sdu = lcp_wantoptions[0].mru + pppoatm_overhead();
	qos.aal = ATM_AAL5;
	if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
		fatal("setsockopt(SO_ATMQOS): %m");

	/* TODO: accept on SVCs... */

	if (connect(fd, (struct sockaddr *) &pvcaddr,
	    sizeof(struct sockaddr_atmpvc)))
		fatal("connect(%s): %m", devnam);
	pppoatm_max_mtu = lcp_allowoptions[0].mru;
	pppoatm_max_mru = lcp_wantoptions[0].mru;
	return fd;
}


static void post_open_setup_pppoatm(int fd)
{
	/* NOTHING */
}

static void pre_close_restore_pppoatm(int fd)
{
	/* NOTHING */
}


static void set_line_discipline_pppoatm(int fd)
{
	struct atm_backend_ppp be;
	be.backend_num = ATM_BACKEND_PPP;

	info("set line dicipline pppoatm\n");
	if (!llc_encaps)
		be.encaps = PPPOATM_ENCAPS_VC;
	else if (!vc_encaps)
		be.encaps = PPPOATM_ENCAPS_LLC;
	else
		be.encaps = PPPOATM_ENCAPS_AUTODETECT;
	if (ioctl(fd, ATM_SETBACKEND, &be) < 0)
		fatal("ioctl(ATM_SETBACKEND): %m");
}


/*
static void reset_line_discipline_pppoatm(int fd)
{
	atm_backend_t be = ATM_BACKEND_RAW;
*/
	/* 2.4 doesn't support this yet */
/*
	(void) ioctl(fd, ATM_SETBACKEND, &be);
}
*/


static void send_config_pppoatm(int unit, int mtu, u_int32_t asyncmap,
	int pcomp, int accomp)
{
	int sock;
	struct ifreq ifr;
	if (mtu > pppoatm_max_mtu)
		error("Couldn't increase MTU to %d", mtu);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		fatal("Couldn't create IP socket: %m");
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_mtu = mtu;
	if (ioctl(sock, SIOCSIFMTU, (caddr_t) &ifr) < 0)
		fatal("ioctl(SIOCSIFMTU): %m");
	(void) close (sock);
}



static void recv_config_pppoatm(int unit, int mru, u_int32_t asyncmap,
	int pcomp, int accomp)
{
	if (mru > pppoatm_max_mru)
		error("Couldn't increase MRU to %d", mru);
}


static void set_xaccm_pppoatm(int unit, ext_accm accm)
{
	/* NOTHING */
}

void pppoatm_phase_change(void *opaque, int PHASE)
{
	int fd;

	switch(PHASE)
	{
		case PHASE_INITIALIZE:
			info("Phase INITIALIZE\n");
			fd = open_device_pppoatm();
			set_line_discipline_pppoatm(fd);
			break;
		case PHASE_SERIALCONN:
			info("Phase SERIALCONN\n");
			break;
		case PHASE_DORMANT:
			info("Phase DORMANT\n");
			break;
		case PHASE_ESTABLISH:
			info("Phase ESTABLISH\n");
			break;
		case PHASE_AUTHENTICATE:
			info("Phase AUTHENTICATE\n");
			break;
		case PHASE_CALLBACK:
			info("Phase CALLBACK\n");
			break;
		case PHASE_NETWORK:
			info("Phase NETWORK\n");
			break;
		case PHASE_RUNNING:
			info("Phase RUNNING\n");
			break;
		case PHASE_TERMINATE:
			info("Phase TERMINATE\n");
			break;
		case PHASE_DISCONNECT:
			info("Phase DISCONNECT\n");
			break;
		case PHASE_HOLDOFF:
			info("Phase HOLDOFF\n");
			break;
	}
	return;
}


static option_t my_options[] = {
#if 0
	{ "accept", o_bool, &pppoatm_accept,
	  "set PPPoATM socket to accept incoming connections", 1 },
#endif
	{ "device name", o_wild, (void *) &setdevname_pppoatm,
	  "Serial port device name",
	  OPT_DEVNAM, OPT_PRIVFIX | OPT_NOARG | OPT_A2STRVAL | OPT_STATIC,
	  devnam},
	{ "llc-encaps", o_bool, &llc_encaps,
	  "use LLC encapsulation for PPPoATM", 1},
	{ "vc-encaps", o_bool, &vc_encaps,
	  "use VC multiplexing for PPPoATM (default)", 1},
	{ "qos", o_string, &qosstr,
	  "set QoS for PPPoATM connection", 1},
	{ NULL }
};

void plugin_init(void)
{
	info("PPPoATM plugin_init");
	add_options(my_options);
	add_notifier(&phasechange,pppoatm_phase_change,0);
	/*add_devname_class(setdevname_pppoatm);
	setspeed_hook = setspeed_pppoatm;
	options_for_device_hook = options_for_pppoatm;*/
	/*open_device_hook = open_device_pppoatm;
	post_open_setup_hook = post_open_setup_pppoatm;
	pre_close_restore_hook = pre_close_restore_pppoatm;
	no_device_given_hook = no_device_given_pppoatm;
	set_line_discipline_hook = set_line_discipline_pppoatm;
	reset_line_discipline_hook = reset_line_discipline_pppoatm;
	send_config_hook = send_config_pppoatm;
	recv_config_hook = recv_config_pppoatm;
	set_xaccm_hook = set_xaccm_pppoatm;
	{
		char **a;
		for (a = bad_options; *a != NULL; a++)
			remove_option(*a);
	}*/
	modem = 0;
	lcp_wantoptions[0].neg_accompression = 0;
	lcp_allowoptions[0].neg_accompression = 0;
	lcp_wantoptions[0].neg_asyncmap = 0;
	lcp_allowoptions[0].neg_asyncmap = 0;
	lcp_wantoptions[0].neg_pcompression = 0;
}

struct channel pppoatm_channel = {
options: my_options,
process_extra_options: &options_for_pppoatm,
check_options: NULL,
connect: &open_device_pppoatm,
disconnect: NULL,
establish_ppp: &set_line_discipline_pppoatm,
disestablish_ppp: NULL ,
send_config: &send_config_pppoatm,
recv_config: &recv_config_pppoatm,
close: NULL,
cleanup: NULL,
};
