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
#include <net/if.h>
#include <linux/if_ppp.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>

#include <string.h>

char pppd_version[] = VERSION;

#define _PATH_ATMOPT _ROOT_PATH "/etc/ppp/options"


static struct sockaddr_atmpvc pvcaddr;
static char *qosstr = NULL;
static int pppoatm_accept = 0;
static bool llc_encaps = 0;
static bool vc_encaps = 0;
static int device_got_set = 0;
static int pppoatm_max_mtu, pppoatm_max_mru;
static struct channel pppoatm_channel;
static int dev_fd;

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
static int set_line_discipline_pppoatm(int fd);



static int setdevname_pppoatm(const char *cp)
{
	struct sockaddr_atmpvc addr;
	int fd;
	extern struct stat devstat;
	/*if (device_got_set) {
		info("device already set");
		return 1;
	}*/
	info("PPPoATM setdevname_pppoatm");
	info("cp = %s\n", cp);
	memset(&addr, 0, sizeof addr);
	if (text2atm(cp, (struct sockaddr *) &addr, sizeof(addr),
	    T2A_PVC | T2A_NAME) < 0)
	{
		info("erreur openning atm socket\n");
		return 0;
	}
	/*if (!dev_set_ok())
		return -1;*/
	memcpy(&pvcaddr, &addr, sizeof pvcaddr);
	strlcpy(devnam, cp, sizeof devnam);
	info("devnam = %s", devnam);
	devstat.st_mode = S_IFSOCK;
	if(the_channel != &pppoatm_channel) {
		info("registering channel\n");
		the_channel = &pppoatm_channel;
		modem = 0;
		lcp_wantoptions[0].neg_accompression = 0;
		lcp_allowoptions[0].neg_accompression = 0;
		lcp_wantoptions[0].neg_asyncmap = 0;
		lcp_allowoptions[0].neg_asyncmap = 0;
		lcp_wantoptions[0].neg_pcompression = 0;
	}
	info("PPPoATM setdevname_pppoatm - SUCCESS");
	device_got_set = 1;
	return 1;
}

static int setspeed_pppoatm(const char *cp)
{
	info("Setspeed pppoatm\n");
	return 0;
}


static void options_for_pppoatm(void)
{  
	char buf[256];

	info("option for pppoatm\n");
	snprintf(buf,256, _PATH_ATMOPT "%s", devnam);
	if(!options_from_file(buf, 0, 0, 1)) exit(EXIT_OPTION_ERROR);
	return;
}


#define pppoatm_overhead() (llc_encaps ? 6 : 2)

static void no_device_given_pppoatm(void)
{
	fatal("No vpi.vci specified");
}


static void close_device_pppoatm(void)
{
	return;
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
	info("pppoatm_max_mtu = %d", pppoatm_max_mtu );
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


int generic_establish_ppp (int fd)
{
    return -1;
}






static int set_line_discipline_pppoatm(int fd)
{
	struct atm_backend_ppp be;
	int x;
	int index;
	int flags;
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
	if(ioctl(fd, PPPIOCGCHAN,&index) == -1)
	{
		error("Couldn't get channel number: %m");
		goto set_disc_err;
	}
	info("using channel %d", index);
	dev_fd = open("/dev/ppp", O_RDWR);
	if(ioctl(dev_fd, PPPIOCATTCHAN,&index) == -1)
	{
		error("Couldn't attach channel number: %d", index);
		goto set_disc_err_close;
	}
	flags = fcntl(dev_fd, F_GETFL);
	if(flags == -1 || fcntl(dev_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		warn("Couldn't set /dev/pp (channel) to non block: %m");
	ifunit = req_unit;
	info("ifunit = %d", ifunit);
	/*x = ioctl( dev_fd, PPPIOCNEWUNIT, &ifunit);
	if(x<0 && req_unit >= 0 && errno == EEXIST)
	{
		warn("Couldn't allocate PPP unit %d as it is already in use");
		ifunit = -1;
		x = ioctl( dev_fd, PPPIOCNEWUNIT, &ifunit);
	}
	if(x < 0)
	{
		error("Couldn't create new ppp unit %m");
		goto set_disc_err_close;
	}
	add_fd(fd);
	if(ioctl(fd, PPPIOCCONNECT, &ifunit) <0)
	{
		error("Couoldn't attach to PPP unit %d: %m", ifunit);
		goto set_disc_err_close;
	}*/
	return fd ;

set_disc_err_close:
	close(fd);
set_disc_err:
	return -1;
}



static void reset_line_discipline_pppoatm(int fd)
{
	atm_backend_t be = ATM_BACKEND_RAW;
	/* 2.4 doesn't support this yet */
	/*(void) ioctl(fd, ATM_SETBACKEND, &be);*/
	close(dev_fd);
	dev_fd = -1;
	if(ifunit >=0 && ioctl(dev_fd, PPPIOCDETACH) <0)
		error("Couldn't release ppp unit: %m");
	remove_fd(dev_fd);
}


static void send_config_pppoatm(int mtu, u_int32_t asyncmap,
	int pcomp, int accomp)
{
	int sock;
	struct ifreq ifr;
	info("send_config_pppoatm\n");
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



static void recv_config_pppoatm(int mru, u_int32_t asyncmap,
	int pcomp, int accomp)
{
	info("recv_config_pppoatm\n");
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
			/*fd = open_device_pppoatm();
			set_line_discipline_pppoatm(fd);*/
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
	  OPT_DEVNAM| OPT_PRIVFIX | OPT_NOARG | OPT_A2STRVAL | OPT_STATIC,
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
	/*add_notifier(&phasechange,pppoatm_phase_change,0);*/
}

static struct channel pppoatm_channel = {
options: my_options,
process_extra_options: &options_for_pppoatm,
check_options: NULL,
connect: &open_device_pppoatm,
disconnect: &close_device_pppoatm,
establish_ppp: &set_line_discipline_pppoatm,
disestablish_ppp: &reset_line_discipline_pppoatm,
send_config: &send_config_pppoatm,
recv_config: &recv_config_pppoatm,
close: NULL,
cleanup: NULL,
};
